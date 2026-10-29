/* ── pe_cdreg: minimal CD-controller register shadow ──────────────── */

#include "pe_spu.h"
#include "pe_cdreg.h"

#include "pe_guest_ram.h"
#include "pe_disc.h"
#include "pe_gpu.h"
#include "pe_irq.h"
#include "pe_irq_delivery.h"
#include <string.h>
#include "game_port.h"
#include "pe_bootstrap.h"
#include "pe_mdec.h"
#include "pe_sdk.h"

/* Four controller registers + one result-mailbox word, power-on 0. */
static uint8_t g_cdreg[PE_CDREG_SIZE];
static uint8_t g_cdreg_mailbox[PE_CDREG_MAILBOX_SIZE];
static uint8_t g_cdreg_bus_control[4];
static uint8_t g_cdreg_dma3[12];
static uint8_t g_response[16],g_response_tag,g_response_mask;
static uint32_t g_response_size,g_response_pos;
static uint8_t g_response_cursor; /* wraps independently of unread-byte count */
static int g_response_mode;
static PeCdDeviceState g_device;
static PE_Disc *g_device_disc;
static uint8_t g_parameters[16],g_parameter_count,g_command,g_command_parameter;
static uint8_t g_phase,g_parameter_error;
static uint32_t g_remaining_cycles;
static uint8_t g_command_parameters[3];
static uint8_t g_sector[PE_DISC_RAW_SECTOR],g_data[2340];
static uint32_t g_data_pos,g_data_size,g_read_cycles,g_target_lba;
static uint32_t g_position_lba; /* last read/seek position, not next read */
static int g_sector_pending,g_target_pending;
static int g_pause_replay; /* resume a paused stream at its last received sector */
/* GetlocL is independent of both the pending sector and accepted data FIFO. */
static uint8_t g_sector_header[8];
static int g_header_valid;
static int g_in_stream_catchup;

static int CdOutputDmaBusy(void)
{
    pe_addr_t reg=PE_LoadU32(0x8009B34Cu);
    if(reg==0x1F801098u) {
        PeMdecState st;
        PE_MDEC_GetState(&st);
        return (st.dma1_chcr&0x01000000u)!=0;
    }
    if(reg && PE_RangeIsRam(reg,4u))
        return (PE_LoadU32(reg)&0x01000000u)!=0;
    return 0;
}
/* Stat: bit1 motor on, bit5 reading; mode.bit4 also asserts IdError bit3
 * (psx-spx CD status). This flag is status, not by itself an INT5 failure. */
static uint8_t CdDeviceStatus(void)
{
    return (g_device.motor_off?0u:2u)|(g_device.reading?0x20u:0u)|
        ((g_device.mode&0x10u)?8u:0u);
}
/* psx-spx (CDROM, Setmode): "If XA-ADPCM (and/or XA-Filter) is enabled via
 * Setmode, then INT1 is generated only for non-ADPCM sectors."  The decoder
 * takes Mode 2 sectors whose submode has audio and real-time (bits 2 and 6),
 * and the CPU path rejects those whenever the filter is on, so such a sector
 * never reaches the data FIFO while either mode bit is set. */
static uint8_t CdBcd(uint32_t v) { return (uint8_t)(((v/10u)<<4)|(v%10u)); }
static int CdSectorIsRoutedAdpcm(void)
{
    return (g_device.mode&0x48u) && g_sector[15]==2u && (g_sector[18]&0x44u)==0x44u;
}
static uint8_t g_audio_pending[4],g_audio_active[4];
static uint8_t g_xa_muted; /* ADPCTL bit0, independent of command Mute */
static uint8_t g_xa_filter[2];   /* audio lane: Setfilter file/channel */
static void CdAudioTap(void)
{
    PE_Spu_CdSector(g_sector,(uint8_t)g_device.mode,g_device.muted!=0 || g_xa_muted!=0,
                    g_xa_filter[0],g_xa_filter[1],g_audio_active);
}
static uint32_t CdSectorCycles(void) { return (g_device.mode&0x80u)?225792u:451584u; }
static void CdClearData(void)
{
    g_sector_pending=0;g_data_pos=g_data_size=0;
}


void PE_CdReg_GetDeviceState(PeCdDeviceState *out) { *out=g_device;out->data_remaining=g_data_size-g_data_pos; }
int PE_CdReg_DeviceEnabled(void) { return g_device.enabled!=0; }
static void CdDeviceBoundary(const char *name,uint32_t value)
{
    Bootstrap_ReturnVoid1(name,"CD_device",value);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}
/* Single-track images have no stored lead-out sector. Exactly reaching their
 * validated extent is the controller's end-of-disc event, not an I/O error.
 * psx-spx CDROM: INT4 DataEnd; at end of DISC the drive STOPs. Keep the
 * last data FIFO intact, and keep genuine in-range read failures fatal. */
static int CdReadNextSector(void)
{
    if(g_device.next_lba==PE_Disc_UserSectorCount(g_device_disc)) {
        uint8_t status;
        g_device.reading=0u;g_device.motor_off=1u;g_read_cycles=0u;
        status=CdDeviceStatus();
        if(!PE_CdReg_PushResponse(4u,&status,1u)) {
            CdDeviceBoundary("CD_device_response_queue",4u);return 0;
        }
        if(g_device.responses<16u) g_device.response_log[g_device.responses]=4u;
        g_device.responses++;
        return 0;
    }
    if(!PE_Disc_ReadRawSector(g_device_disc,g_device.next_lba,g_sector)) {
        CdDeviceBoundary("CD_device_sector_read",g_device.next_lba);return 0;
    }
    memcpy(g_sector_header,g_sector+12u,sizeof(g_sector_header));g_header_valid=1;
    g_position_lba=g_device.next_lba;
    return 1;
}
int PE_CdReg_EnableDevice(uint8_t interrupt_mask)
{
    if(!PE_Disc_GetActive() || g_response_tag || g_response_pos<g_response_size || g_device.enabled) return 0;
    g_device=(PeCdDeviceState){0};g_device.enabled=1u;g_device.mode=0x20u;
    g_device_disc=PE_Disc_GetActive();g_phase=0;g_parameter_count=0;
    CdClearData();g_read_cycles=0;g_target_lba=0;g_target_pending=0;g_header_valid=0;g_position_lba=0;g_pause_replay=0;
    g_response_mode=1;g_response_mask=interrupt_mask&31u;
    return 1;
}
static void CdDeviceCommand(uint8_t command)
{
    if(g_phase) {
        if(command==10u && g_command==10u) {g_parameter_count=0;return;}
        /* Init aborts an earlier command, including a pending second
         * response. An already published response still belongs to the
         * consumer: the new Init response waits for its acknowledgement.
         * Malformed/interleaved non-Init commands remain unsupported. */
        if(command!=10u || g_parameter_count) {
            CdDeviceBoundary("CD_device_busy_command",command);return;
        }
    }
    if(command!=2u && command!=6u && command!=9u && command!=21u && command!=22u && command!=27u && command!=1u && command!=10u && command!=11u && command!=12u && command!=14u && command!=15u && command!=19u &&
       command!=7u && command!=8u && command!=13u && command!=16u && command!=17u && command!=20u) {
        CdDeviceBoundary("CD_device_unported_command",command);return;
    }
    if(g_device.commands<16u) g_device.command_log[g_device.commands]=command;
    g_device.commands++;g_command=command;
    /* Setloc mm,ss,ff; Setfilter file,channel; Setmode mode. */
    g_parameter_error=g_parameter_count!=(command==2u?3u:command==13u?2u:(command==14u || command==20u)?1u:0u);
    /* Reads, seeks and Init spin a stopped motor back up ("almost any
     * command will restart it" -- psx-spx names Init as the official way).
     * A rejected parameter list must not perform that command side effect. */
    if(!g_parameter_error &&
       (command==6u || command==27u || command==10u || command==21u || command==22u))
        g_device.motor_off=0u;
    for(unsigned i=0;i<3u;i++) g_command_parameters[i]=i<g_parameter_count?g_parameters[i]:0u;
    g_command_parameter=g_parameter_count?g_parameters[0]:0u;g_parameter_count=0;
    g_phase=1;g_remaining_cycles=command==10u?0x13CCEu:0xC4E1u;
}
uint32_t PE_CdReg_TimeSlice(uint32_t budget)
{
    if(!g_device.enabled) return budget;
    if(g_phase && g_remaining_cycles && g_remaining_cycles<budget)
        budget=g_remaining_cycles;
    if(g_device.reading && g_read_cycles && g_read_cycles<budget)
        budget=g_read_cycles;
    return budget;
}
void PE_CdReg_ServiceDevice(uint32_t elapsed_cycles)
{
    if(!g_device.enabled || PE_Port_ShouldStop()) return;
    if(PE_Disc_GetActive()!=g_device_disc) {CdDeviceBoundary("CD_device_media_changed",g_command);return;}
    /* Retry a B0CD0 defer as soon as output DMA is idle, even if an INT1
     * result is still sitting in the response mailbox. */
    if(!g_in_stream_catchup && g_sector_pending &&
       (int16_t)PE_LoadU16(0x800B0CD0u) && PE_LoadU32(0x800A801Cu) &&
       !CdOutputDmaBusy()) {
        g_in_stream_catchup=1;
        func_8007C564();
        g_in_stream_catchup=0;
        if(PE_Port_ShouldStop()) return;
    }
    /* Read cadence is separate from command acknowledgments. A response
     * generated during this service cannot consume the same elapsed time twice. */
    int was_reading=g_device.reading;
    if(was_reading) g_read_cycles=elapsed_cycles>=g_read_cycles?0u:g_read_cycles-elapsed_cycles;
    if(g_phase) {
        g_remaining_cycles=elapsed_cycles>=g_remaining_cycles?0u:g_remaining_cycles-elapsed_cycles;
        if(!g_remaining_cycles && !g_response_tag && g_response_pos==g_response_size) {
            uint8_t response[8]={CdDeviceStatus(),0u,0u,0u,0u,0u,0u,0u};uint32_t size=1u;uint8_t tag=3u;
            if(g_phase==2u) {
                tag=2u;g_phase=0;
                if(g_command==8u) {
                    g_device.motor_off=1u;
                    /* Stop parks at the first track; in the mounted single
                     * data-track format this is LBA0. Setloc's unconsumed
                     * logical target and accepted data FIFO are separate. */
                    g_device.next_lba=0u;g_position_lba=0u;
                }
                if(g_command==21u || g_command==22u) {
                    if(g_command==21u) {
                        /* Logical seeks inspect data headers, but must not
                         * replace a sector still waiting for BFRD/DMA. The
                         * existing seek model lands exactly at the target. */
                        uint8_t sector[PE_DISC_RAW_SECTOR];
                        if(!PE_Disc_ReadRawSector(g_device_disc,g_target_lba,sector)) {
                            CdDeviceBoundary("CD_device_seek_header",g_target_lba);return;
                        }
                        memcpy(g_sector_header,sector+12u,sizeof(g_sector_header));g_header_valid=1;
                    }
                    g_device.next_lba=g_target_lba;g_target_pending=0;g_position_lba=g_target_lba;
                }
                response[0]=CdDeviceStatus();
            }
            else if(g_parameter_error) {response[0]|=1u;response[1]=0x20u;size=2u;tag=5u;g_phase=0;}
            else {
                g_phase=0;
                if(g_command==10u) {
                    g_device.mode=0x20u;g_device.muted=0;g_device.reading=0;CdClearData();g_header_valid=0;g_pause_replay=0;
                    response[0]=CdDeviceStatus(); /* Init's first response is late status. */
                    g_phase=2;g_remaining_cycles=0x100000u;
                } else if(g_command==7u) {
                    /* MotorOn is not Pause: an already-running motor
                     * returns INT5/20h without interrupting a read. Only a
                     * stopped motor starts the two-response sequence.
                     * Spin-up uses the existing approximate phase delay. */
                    if(!g_device.motor_off) {
                        response[0]|=1u;response[1]=0x20u;size=2u;tag=5u;
                    } else {
                        g_device.motor_off=0u;
                        g_phase=2;g_remaining_cycles=0x100000u;
                    }
                } else if(g_command==2u) {
                    unsigned v[3];int valid=1;
                    for(unsigned i=0;i<3;i++) {
                        unsigned b=g_command_parameters[i];
                        if((b&15u)>9u || (b>>4u)>9u) valid=0;
                        v[i]=(b>>4u)*10u+(b&15u);
                    }
                    if(v[1]>=60u || v[2]>=75u) valid=0;
                    uint32_t frame=(v[0]*60u+v[1])*75u+v[2];
                    /* Pre-gap and outside-image locations remain explicit boundaries. */
                    if(valid && (frame<150u || frame-150u>=PE_Disc_UserSectorCount(g_device_disc))) {
                        CdDeviceBoundary("CD_device_location_range",frame);return;
                    }
                    if(!valid) {response[0]|=1u;response[1]=0x10u;size=2;tag=5;}
                    else {g_target_lba=frame-150u;g_target_pending=1;}
                } else if(g_command==6u || g_command==27u) {
                    /* Every read mode is served from the raw sector: mode bit5
                     * selects the 2048/2340-byte FIFO window, and double speed
                     * (0x80) and ignore (0x10) do not change the bytes handed
                     * over.  XA-ADPCM (0x40) and the XA filter (0x08) decide
                     * which sectors are handed over at all: audio+real-time
                     * sectors go to the SPU instead (CdSectorIsRoutedAdpcm).
                     * The movie stream opens ReadS with mode 0x1E0 (double
                     * speed + XA + 2340); the credits with 0xC8. */
                    /* A repeated Read without Setloc keeps the running
                     * cadence. After Pause, it replays the last received
                     * sector. A pending explicit target takes precedence. */
                    int restart=!g_device.reading || g_target_pending;
                    if(g_target_pending) {g_device.next_lba=g_target_lba;g_target_pending=0;g_header_valid=0;}
                    else if(!g_device.reading && g_pause_replay)
                        g_device.next_lba=g_position_lba;
                    g_pause_replay=0;g_device.reading=1;
                    if(restart) g_read_cycles=CdSectorCycles();
                } else if(g_command==9u || g_command==21u || g_command==22u || g_command==8u) {
                    /* Stop's first INT3 already has bit5 clear; its INT2
                     * clears bit1 and parks the head. Pause depends on
                     * reading state, Stop on motor state (a paused drive
                     * still spins). Use psx-spx's measured average
                     * delays (CPU cycles), not one delay for all speeds.
                     * These are deterministic approximations, not a model
                     * of the drive's mechanical timing variation. */
                    if(g_command==9u)
                        g_remaining_cycles=g_device.reading
                            ?((g_device.mode&0x80u)?0x10BD93u:0x21181Cu)
                            :0x1DF2u;
                    else if(g_command==8u)
                        g_remaining_cycles=g_device.motor_off?0x1D7Bu
                            :((g_device.mode&0x80u)?0x18A6076u:0xD38ACAu);
                    else g_remaining_cycles=0x100000u; /* seek approximation */
                    if(g_command==9u) {
                        if(g_device.reading) g_pause_replay=g_header_valid;
                    } else g_pause_replay=0;
                    g_device.reading=0;g_phase=2;
                    if(g_command!=9u) g_header_valid=0;
                    if(g_command==8u) response[0]=CdDeviceStatus();
                } else if(g_command==16u) {
                    /* Raw header + first subheader, no status prefix. */
                    if(!g_header_valid || g_device.motor_off) {
                        response[0]|=1u;response[1]=0x80u;size=2u;tag=5u;
                    } else {
                        memcpy(response,g_sector_header,sizeof(g_sector_header));size=8u;
                    }
                } else if(g_command==17u) {
                    /* GetlocP: INT3(track,index,mm,ss,sect,amm,ass,asect),
                     * BCD, no stat byte.  The one data track starts at
                     * 00:02:00, so the in-track position is the LBA and the
                     * disc position LBA + 150. */
                    uint32_t lba=g_position_lba,abs=lba+150u,rel=lba;
                    if(g_device.motor_off) {CdDeviceBoundary("CD_device_getlocp_spun_down",lba);return;}
                    response[0]=1u;response[1]=1u;
                    response[2]=CdBcd(rel/4500u);response[3]=CdBcd(rel/75u%60u);response[4]=CdBcd(rel%75u);
                    response[5]=CdBcd(abs/4500u);response[6]=CdBcd(abs/75u%60u);response[7]=CdBcd(abs%75u);
                    size=8u;
                }
                else if(g_command==11u) g_device.muted=1u;
                else if(g_command==12u) g_device.muted=0u;
                else if(g_command==14u) g_device.mode=g_command_parameter;
                else if(g_command==13u) {g_xa_filter[0]=g_command_parameters[0];g_xa_filter[1]=g_command_parameters[1];}
                else if(g_command==15u) {
                    /* Getparam returns mode, reserved zero, file, channel. */
                    response[1]=g_device.mode;
                    response[3]=g_xa_filter[0];response[4]=g_xa_filter[1];size=5u;
                }
                else if(g_command==19u) {response[1]=response[2]=1u;size=3u;}
                else if(g_command==20u) {
                    /* Mounted format is one MODE2/2352 track, INDEX 01 at
                     * 00:02:00. Track zero requests the lead-out; GetTD
                     * truncates frames and returns BCD minutes/seconds. */
                    if(g_command_parameter>1u) {
                        response[0]|=1u;response[1]=0x10u;size=2u;tag=5u;
                    } else {
                        uint64_t frame=150u+(g_command_parameter==0u
                            ?(uint64_t)PE_Disc_UserSectorCount(g_device_disc):0u);
                        if(frame/4500u>99u) {
                            CdDeviceBoundary("CD_device_gettd_extent",PE_Disc_UserSectorCount(g_device_disc));return;
                        }
                        response[1]=CdBcd((uint32_t)(frame/4500u));
                        response[2]=CdBcd((uint32_t)(frame/75u%60u));size=3u;
                    }
                }
            }
            if(!PE_CdReg_PushResponse(tag,response,size)) {CdDeviceBoundary("CD_device_response_queue",tag);return;}
            if(g_device.responses<16u) g_device.response_log[g_device.responses]=tag;
            g_device.responses++;
        }
    }
    if(was_reading && g_device.reading && !g_read_cycles) {
        if(!g_phase && !g_response_tag && g_response_pos==g_response_size) {
            if(g_sector_pending && (int16_t)PE_LoadU16(0x800B0CD0u)) {
                /* Preserve the existing MDEC output-DMA deferral contract. */
                if(!g_in_stream_catchup && PE_LoadU32(0x800A801Cu) &&
                   !CdOutputDmaBusy()) {
                    g_in_stream_catchup=1;
                    func_8007C564();
                    g_in_stream_catchup=0;
                    if(PE_Port_ShouldStop()) return;
                }
                if(g_sector_pending) {
                    g_read_cycles=CdSectorCycles();
                    goto service_done;
                }
            }
            /* Once INT1 is acknowledged, unread data may be superseded.
             * The FIFO already accepted with BFRD is separate (g_data). */
            if(!CdReadNextSector()) goto service_done;
            CdAudioTap();
            g_device.next_lba++;g_device.sectors++;g_read_cycles=CdSectorCycles();
            g_sector_pending=0;
            if(CdSectorIsRoutedAdpcm()) g_device.xa_sectors++;
            else {
                g_sector_pending=1;
                uint8_t status=CdDeviceStatus();
                if(!PE_CdReg_PushResponse(1u,&status,1u)) {CdDeviceBoundary("CD_device_response_queue",1u);return;}
                if(g_device.responses<16u) g_device.response_log[g_device.responses]=1u;
                g_device.responses++;
            }
        }
    }
service_done:
    if(PE_Port_ShouldStop()) return;
    PE_CdReg_ServiceDMA3();
    if(g_response_tag&g_response_mask) PE_IRQ_AssertSources(4u);
}

int PE_CdReg_XaMuted(void) { return g_xa_muted!=0; }

void PE_CdReg_GetAudioVolumes(uint8_t out[4])
{
    for(unsigned i=0;i<4;i++) out[i]=g_audio_active[i];
}

int PE_CdReg_PushResponse(uint8_t tag,const uint8_t *bytes,uint32_t count)
{
    if(!tag || tag>7u || count>sizeof(g_response) || (count && !bytes) ||
       g_response_tag || g_response_pos<g_response_size) return 0;
    memset(g_response,0,sizeof(g_response));
    for(uint32_t i=0;i<count;i++) g_response[i]=bytes[i];
    g_response_cursor=0u;
    g_response_pos=0u;g_response_size=count;g_response_tag=tag;
    g_response_mode=1;
    return 1;
}

int PE_CdReg_IsMmio(pe_addr_t address, uint32_t size)
{
    uint64_t end = (uint64_t)address + (uint64_t)size;
    if (size == 0u || end > 0xFFFFFFFFu)
        return 0;
    if (address >= PE_CDREG_BASE &&
        end <= (uint64_t)PE_CDREG_BASE + PE_CDREG_SIZE)
        return 1;
    if (address >= PE_CDREG_MAILBOX &&
        end <= (uint64_t)PE_CDREG_MAILBOX + PE_CDREG_MAILBOX_SIZE)
        return 1;
    if (address >= PE_CDREG_BUS_CONTROL &&
        end <= (uint64_t)PE_CDREG_BUS_CONTROL + 4u)
        return 1;
    if (address >= PE_CDREG_DMA3 && end <= (uint64_t)PE_CDREG_DMA3+12u)
        return 1;
    return 0;
}

void PE_CdReg_Reset(void)
{
    uint32_t i;
    g_response_mode=0;g_response_tag=0u;g_response_mask=0u;
    g_response_pos=0u;g_response_size=0u;g_response_cursor=0u;
    memset(g_response,0,sizeof(g_response));
    g_device=(PeCdDeviceState){0};g_device_disc=NULL;g_phase=0;g_parameter_count=0;g_remaining_cycles=0;
    CdClearData();g_read_cycles=0;g_target_lba=0;g_target_pending=0;g_header_valid=0;g_position_lba=0;g_pause_replay=0;
    for(unsigned i=0;i<4;i++) g_audio_pending[i]=g_audio_active[i]=0u;
    g_xa_filter[0]=g_xa_filter[1]=0u;g_xa_muted=0;
    for (i = 0u; i < PE_CDREG_SIZE; i++)
        g_cdreg[i] = 0u;
    for (i = 0u; i < PE_CDREG_MAILBOX_SIZE; i++)
        g_cdreg_mailbox[i] = 0u;
    for (i = 0u; i < 4u; i++)
        g_cdreg_bus_control[i] = 0u;
    for (i = 0u; i < 12u; i++)
        g_cdreg_dma3[i] = 0u;
}

/* End a mid-game attachment (PE_Cd_DetachDevice).  Like PE_CdReg_Reset, but
 * the CD audio mix latch (ATV0..3 pending/applied and ADPMUTE) survives: it is the
 * game's own CdMix state (func_8007A88C / func_800870F0).  pe_cdreg keeps it
 * with the device disabled too, and the host movie feed reads it there
 * (pe_str_feed.c PE_CdReg_GetAudioVolumes).  Retail's mix after the credits
 * is whatever the credits last applied, not zero. */
void PE_CdReg_DetachDevice(void)
{
    uint8_t pending[4],active[4],xa_muted=g_xa_muted;
    memcpy(pending,g_audio_pending,4);memcpy(active,g_audio_active,4);
    PE_CdReg_Reset();
    memcpy(g_audio_pending,pending,4);memcpy(g_audio_active,active,4);g_xa_muted=xa_muted;
}

static uint8_t *cdreg_byte(pe_addr_t address)
{
    if (address >= PE_CDREG_DMA3 && address < PE_CDREG_DMA3+12u)
        return &g_cdreg_dma3[address-PE_CDREG_DMA3];
    if (address >= PE_CDREG_BUS_CONTROL && address < PE_CDREG_BUS_CONTROL+4u)
        return &g_cdreg_bus_control[address-PE_CDREG_BUS_CONTROL];
    if (address >= PE_CDREG_BASE &&
        address < PE_CDREG_BASE + PE_CDREG_SIZE)
        return &g_cdreg[address - PE_CDREG_BASE];
    return &g_cdreg_mailbox[address - PE_CDREG_MAILBOX];
}

uint8_t PE_CdReg_ReadU8(pe_addr_t address)
{
    if(g_device.enabled && address==PE_CDREG_BASE+2u) {
        if(g_data_pos==g_data_size) {
            /* RDDATA over-read repeats a byte near the accepted window's
             * end, without making DRQ ready again or advancing the drive. */
            if(g_data_size) return g_data[g_data_size-(g_data_size==2048u?8u:4u)];
            CdDeviceBoundary("CD_device_data_underflow",g_data_pos);return 0;
        }
        return g_data[g_data_pos++];
    }
    if(g_response_mode) {
        if(address==PE_CDREG_BASE)
            return (g_cdreg[0]&3u)|(g_device.enabled && g_data_pos<g_data_size?0x40u:0u)|(g_response_pos<g_response_size?0x20u:0u)|
                (g_device.enabled?((g_parameter_count?0u:8u)|(g_parameter_count<16u?16u:0u)|(g_phase==1u?0x80u:0u)):0u);
        if(address==PE_CDREG_BASE+1u) {
            /* RESULT repeats its padded 16-byte buffer on over-read. The
             * ready bit tracks unread response bytes, not this ring cursor. */
            uint8_t result=g_response[g_response_cursor];
            g_response_cursor=(g_response_cursor+1u)&15u;
            if(g_response_pos<g_response_size) g_response_pos++;
            return result;
        }
        if(address==PE_CDREG_BASE+3u)
            return 0xE0u|((g_cdreg[0]&1u)?g_response_tag:g_response_mask);
    }
    return *cdreg_byte(address);
}

void PE_CdReg_WriteU8(pe_addr_t address, uint8_t value)
{
    if(g_device.enabled && (g_cdreg[0]&3u)==0u) {
        if(address==PE_CDREG_BASE+2u) {
            if(g_parameter_count==16u) {CdDeviceBoundary("CD_device_parameter_overflow",value);return;}
            g_parameters[g_parameter_count++]=value;
        } else if(address==PE_CDREG_BASE+1u) CdDeviceCommand(value);
    }
    if(g_device.enabled && address==PE_CDREG_BASE+3u && (g_cdreg[0]&3u)==0u) {
        if(value&0x60u) {CdDeviceBoundary("CD_device_buffer_write",value);return;}
        if(!(value&0x80u)) g_data_pos=g_data_size=0;
        else if(g_sector_pending) {
            /* BFRD (want data, bit7) loads the pending sector into the FIFO.
             * Hardware discards any unread tail of the previous sector when the
             * request is reasserted, so the stream reader's partial 2060-byte
             * consumption (12-byte header + 32-byte chunk header + 2016-byte
             * body of a 2340-byte window) never blocks the next sector. */
            uint32_t offset=(g_device.mode&0x20u)?12u:24u;
            g_data_size=(g_device.mode&0x20u)?2340u:2048u;g_data_pos=0;
            for(unsigned i=0;i<g_data_size;i++) g_data[i]=g_sector[offset+i];
            g_sector_pending=0;
        }
    }
    /* ATV0..3 are pending banked volumes; ADPCTL bit5 applies all four.
     * This owns register state only; audio sample mixing is separate. */
    unsigned bank=g_cdreg[0]&3u;
    if(bank==2u && address==PE_CDREG_BASE+2u) g_audio_pending[0]=value;
    if(bank==2u && address==PE_CDREG_BASE+3u) g_audio_pending[1]=value;
    if(bank==3u && address==PE_CDREG_BASE+1u) g_audio_pending[2]=value;
    if(bank==3u && address==PE_CDREG_BASE+2u) g_audio_pending[3]=value;
    if(bank==3u && address==PE_CDREG_BASE+3u) {
        g_xa_muted=value&1u;
        if(value&0x20u)
            for(unsigned i=0;i<4;i++) g_audio_active[i]=g_audio_pending[i];
    }
    if(g_response_mode && address==PE_CDREG_BASE+3u && (g_cdreg[0]&3u)==1u) {
        g_response_tag&=(uint8_t)~(value&7u);
        if(g_device.enabled) {
            if(value&0x40u) g_parameter_count=0;
            if(!g_response_tag) g_response_pos=g_response_size;
        }
        return;
    }
    if(g_response_mode && address==PE_CDREG_BASE+2u && (g_cdreg[0]&3u)==1u) {
        g_response_mask=value&31u;
        return;
    }
    *cdreg_byte(address) = value;
}

static void CdDmaStore(unsigned offset,uint32_t value)
{
    for(unsigned i=0;i<4;i++) g_cdreg_dma3[offset+i]=(uint8_t)(value>>(i*8u));
}
void PE_CdReg_ServiceDMA3(void)
{
    if(!g_device.enabled || PE_Port_ShouldStop()) return;
    uint32_t control=PE_CdReg_ReadU32(PE_CDREG_DMA3+8u);
    if(!(control&0x01000000u) || !(PE_GPU_ReadDPCR()&0x8000u)) return;
    uint32_t mode=control&~2u;
    if(mode!=0x11000000u && mode!=0x11400100u) {
        CdDeviceBoundary("CD_dma3_control",control);return;
    }
    if(g_data_pos==g_data_size) return; /* wait for the device request */
    uint32_t address=PE_CdReg_ReadU32(PE_CDREG_DMA3)&0x00FFFFFCu;
    uint32_t bcr=PE_CdReg_ReadU32(PE_CDREG_DMA3+4u);
    uint32_t count=(bcr&0xFFFFu)?(bcr&0xFFFFu):65536u,bytes=count*4u;
    int decrement=(control&2u)!=0;
    if(address>=PE_RAM_SIZE || (decrement?bytes-4u>address:bytes>PE_RAM_SIZE-address)) {
        CdDeviceBoundary("CD_dma3_ram_range",address);return;
    }
    /* Burst DMA uses the same 8-bit device bus as CPU RDDATA reads. Once
     * started, an over-read pads from the accepted sector's documented
     * repeat byte; it does not fetch the following sector or overrun g_data.
     * RAM bounds were admitted above before consuming any FIFO bytes. */
    for(unsigned word=0;word<count;word++) {
        pe_addr_t destination=PE_RAM_BASE+address+(decrement?0u-word*4u:word*4u);
        for(unsigned byte=0;byte<4u;byte++)
            PE_StoreU8(destination+byte,PE_CdReg_ReadU8(PE_CDREG_BASE+2u));
    }
    if(control&0x100u) {
        CdDmaStore(0u,(address+(decrement?0u-bytes:bytes))&0xFFFFFFu);
        CdDmaStore(4u,bcr&0xFFFF0000u);
    }
    CdDmaStore(8u,control&~0x11000000u);
    (void)PE_GPU_LatchDMACompletionFlag(3u);
    (void)PE_IRQ_BridgeDICRRisingEdge(PE_IRQ_Generation());
}

uint32_t PE_CdReg_ReadU32(pe_addr_t address)
{
    uint32_t i;
    uint32_t v = 0u;
    for (i = 0u; i < 4u; i++)
        v |= (uint32_t)cdreg_byte(address + i)[0] << (i * 8u);
    return v;
}

void PE_CdReg_WriteU32(pe_addr_t address, uint32_t value)
{
    uint32_t i;
    if(g_device.enabled && address==PE_CDREG_DMA3) value&=0xFFFFFFu;
    for (i = 0u; i < 4u; i++)
        cdreg_byte(address + i)[0] = (uint8_t)((value >> (i * 8u)) & 0xFFu);
    if(address==PE_CDREG_DMA3+8u) PE_CdReg_ServiceDMA3();
}

uint8_t PE_CdLoadU8(pe_addr_t address)
{
    if (PE_CdReg_IsMmio(address, 1u))
        return PE_CdReg_ReadU8(address);
    return PE_LoadU8(address);
}

void PE_CdStoreU8(pe_addr_t address, uint8_t value)
{
    if (PE_CdReg_IsMmio(address, 1u)) {
        PE_CdReg_WriteU8(address, value);
        return;
    }
    PE_StoreU8(address, value);
}

uint32_t PE_CdLoadU32(pe_addr_t address)
{
    if (PE_CdReg_IsMmio(address, 4u))
        return PE_CdReg_ReadU32(address);
    return PE_LoadU32(address);
}

void PE_CdStoreU32(pe_addr_t address, uint32_t value)
{
    if (PE_CdReg_IsMmio(address, 4u)) {
        PE_CdReg_WriteU32(address, value);
        return;
    }
    PE_StoreU32(address, value);
}
