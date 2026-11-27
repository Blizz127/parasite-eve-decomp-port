/* Movie overlay at LBA1978, SHA256
 * 5ddd18d8a7f2a8180f92c1c9c072996e9705605e2daac8dc02a665a35f470ec0.
 * 1216C4..121A00: complete buffer/display initializer.
 * 1223A8..1223F4: complete completion-request flag helper.
 * These entry points do not implement the movie player121C04. */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "pe_plat/mdec_hw.h"
#include "pe_plat/cd_regs.h"
#include "pe_plat/framebuffer_api.h"

extern int func_8007C484(pe_addr_t data, pe_addr_t header);

/* 1214D4..1216C4: DMA1 movie slice callback. Supplied decode commands
 * queue the next output; missing input and stream boundaries propagate. */
void func_801214D4(void)
{
    RECT saved;
    uint32_t old_buffer, next_buffer, bank;
    int32_t right;
    if ((int8_t)PE_LoadU8(0x800B0DBBu) &&
        (int16_t)PE_LoadU16(0x800B0CD0u)) {
        func_8007C564();
        if (PE_Port_ShouldStop()) return;
        PE_StoreU16(0x800B0CD0u,0u);
    }
    saved.x=(int16_t)PE_LoadU16(0x801228F4u);
    saved.y=(int16_t)PE_LoadU16(0x801228F6u);
    saved.w=(int16_t)PE_LoadU16(0x801228F8u);
    saved.h=(int16_t)PE_LoadU16(0x801228FAu);
    old_buffer=PE_LoadU8(0x801228E0u);
    next_buffer=old_buffer^1u;
    PE_StoreU8(0x801228E0u,(uint8_t)next_buffer);
    bank=PE_LoadU8(0x801228F2u);
    PE_StoreU16(0x801228F4u,(uint16_t)((uint16_t)saved.x+(uint16_t)saved.w));
    right=(int16_t)PE_LoadU16(0x801228E2u+bank*8u)+
          (int16_t)PE_LoadU16(0x801228E6u+bank*8u);
    if ((int16_t)PE_LoadU16(0x801228F4u)<right) {
        int32_t words=((int32_t)saved.w*(int32_t)saved.h)/2;
        if(!PE_MDEC_HasDecode()) {
            Bootstrap_ReturnVoid4("func_8010C01C", "func_801214D4",
                PE_LoadU32(0x801228D8u+next_buffer*4u),(uint32_t)words,0u,0u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return;
        }
        func_8010C01C(PE_LoadU32(0x801228D8u+next_buffer*4u),(uint32_t)words);
        if(PE_Port_ShouldStop()) return;
    } else {
        PE_StoreU8(0x801228FCu,1u);
        bank^=1u;
        PE_StoreU8(0x801228F2u,(uint8_t)bank);
        PE_StoreU16(0x801228F4u,PE_LoadU16(0x801228E2u+bank*8u));
        PE_StoreU16(0x801228F6u,PE_LoadU16(0x801228E4u+bank*8u));
        if (PE_LoadU8(0x801223F8u)==1u) {
            uint32_t wide=PE_LoadU8(0x800B0DBBu)^1u;
            PE_StoreU8(0x800B0DBBu,(uint8_t)wide);
            PE_StoreU16(0x801228F8u,wide?24u:16u);
            func_80121004((int8_t)(PE_LoadU32(0x8009CDDCu)^1u),
                          (int8_t)wide);
            if (PE_Port_ShouldStop()) return;
            PE_StoreU8(0x801223F8u,2u);
        }
    }
    func_8007506C(&saved,PE_LoadU32(0x801228D8u+
                                   (uint32_t)(int32_t)(int8_t)old_buffer*4u));
}

int func_801216C4(int mode, pe_addr_t buffers)
{
    unsigned count=(uint8_t)mode;
    pe_addr_t first,second;
    RECT rect;
    if ((uint8_t)(count-1u)>=2u) return 0;
    for (unsigned i=0;i<count;i++)
        if (!PE_LoadU32(buffers+i*4u)) return 0;
    if (PE_LoadU8(0x800B0DBAu)) return 0;
    first=PE_LoadU32(buffers);
    PE_StoreU32(0x80122420u,first);
    PE_StoreU32(0x80122424u,first+0xFA00u);
    PE_StoreU32(0x80122434u,first+0x1F400u);
    if (count==1u) {
        PE_StoreU32(0x80122428u,first+0x50400u);
        PE_StoreU32(0x80122430u,first+0x3F400u);
        PE_StoreU32(0x8012242Cu,first+0x53100u);
    } else {
        second=PE_LoadU32(buffers+4u);
        PE_StoreU32(0x80122428u,second+0x11000u);
        PE_StoreU32(0x80122430u,second);
        PE_StoreU32(0x8012242Cu,second+0x13D00u);
    }
    PE_StoreU8(0x800B0DBAu,1u);
    PE_StoreU8(0x800B0DBEu,0x98u);
    PE_StoreU16(0x800B0DBCu,0u);
    PE_StoreU16(0x801227E8u,0xFFFFu);
    for (unsigned bank=0;bank<2;bank++) {
        for (unsigned offset=0;offset<20;offset+=4)
            PE_StoreU32(0x801227ECu+bank*20u+offset,
                        PE_LoadU32(0x800BCE80u+bank*20u+offset));
        for (unsigned offset=0;offset<92;offset+=4)
            PE_StoreU32(0x80122814u+bank*92u+offset,
                        PE_LoadU32(0x800BCDC8u+bank*92u+offset));
    }
    rect.x=320;rect.y=0;rect.w=192;rect.h=256;
    func_8007512C(&rect,512,0);
    if (PE_Port_ShouldStop()) return 0;
    if (!(PE_LoadU32(0x800B0CD8u)&0x08000000u)) {
        rect.x=0;rect.y=448;rect.w=320;rect.h=64;
        func_8007512C(&rect,512,256);
        if (PE_Port_ShouldStop()) return 0;
    }
    return 1;
}

void func_801223A8(int mode)
{
    int active=(int8_t)PE_LoadU8(0x800B0DBBu);
    if (((uint8_t)mode!=0 && !active) || ((uint8_t)mode==0 && active))
        PE_StoreU8(0x801223F8u,1u);
}

/* 121A00..121C04: restore saved display state and VRAM after a movie. */
void func_80121A00(void)
{
    RECT rect;
    if (!PE_LoadU8(0x800B0DBAu)) return;
    PE_StoreU32(0x80122420u,0u);
    PE_StoreU32(0x80122424u,0u);
    PE_StoreU32(0x80122434u,0u);
    PE_StoreU32(0x80122430u,0u);
    PE_StoreU32(0x80122428u,0u);
    PE_StoreU32(0x8012242Cu,0u);
    PE_StoreU8(0x800B0DBAu,0u);
    func_80074DC0(0);
    if (PE_Port_ShouldStop()) return;
    func_80073A44(0);
    if (PE_Port_ShouldStop()) return;
    func_80074D28(0);
    if (PE_Port_ShouldStop()) return;
    for (unsigned bank=0;bank<2;bank++) {
        for (unsigned offset=0;offset<20;offset+=4)
            PE_StoreU32(0x800BCE80u+bank*20u+offset,
                        PE_LoadU32(0x801227ECu+bank*20u+offset));
        for (unsigned offset=0;offset<92;offset+=4)
            PE_StoreU32(0x800BCDC8u+bank*92u+offset,
                        PE_LoadU32(0x80122814u+bank*92u+offset));
    }
    rect.x=512;rect.y=0;rect.w=192;rect.h=256;
    func_8007512C(&rect,320,0);
    if (PE_Port_ShouldStop()) return;
    if (!(PE_LoadU32(0x800B0CD8u)&0x08000000u)) {
        rect.x=512;rect.y=256;rect.w=320;rect.h=64;
        func_8007512C(&rect,0,448);
    }
}

/* 121004..121270: configure one movie display/draw environment. */
void func_80121004(int buffer, int wide)
{
    unsigned stop_epoch=PE_Port_StopEpoch();
    int index=(int8_t)buffer;
    int display_y=(uint8_t)buffer?0:240;
    int draw_y=(uint8_t)buffer?240:0;
    pe_addr_t disp=0x800BCE80u+(uint32_t)(index*20);
    pe_addr_t draw=0x800BCDC8u+(uint32_t)(index*92);
    int width=(uint8_t)wide?480:320;
    PE_StoreU8(0x801223F6u,(uint8_t)wide?3u:2u);
    func_800749D8(disp,0,display_y,width,240);
    if(PE_Port_StopEpoch()!=stop_epoch) return;
    if ((uint8_t)wide) {
        PE_StoreU8(disp+17u,1u);
        PE_StoreU16(disp+4u,(uint16_t)(((int16_t)PE_LoadU16(disp+4u)*2)/3));
    }
    func_80074924(draw,0,draw_y,width,240);
    if(PE_Port_StopEpoch()!=stop_epoch) return;
    PE_StoreU8(draw+24u,1u);PE_StoreU8(draw+22u,1u);
    PE_StoreU8(draw+23u,0u);PE_StoreU8(draw+25u,0u);
    PE_StoreU8(draw+26u,0u);PE_StoreU8(draw+27u,0u);
    if ((uint8_t)wide)
        PE_StoreU16(draw+4u,(uint16_t)(((int16_t)PE_LoadU16(draw+4u)*2)/3));
}

/* 121270..1214D4. The two address-output arguments use the same explicit
 * host scratch cells as the existing frame-poll adapter, outside live data.
 * No completion is fabricated: 7C484's result and published addresses drive
 * all state updates. Guest stack residue is not part of this host adaptation. */
int func_80121270(pe_addr_t state)
{
    const pe_addr_t out=0x801FFF20u,header_out=0x801FFF24u;
    pe_addr_t header,record;
    uint32_t frame;
    int32_t limit;
    if (PE_Port_ShouldStop()) return 0;
    int ready=func_8007F72C();
    if (PE_Port_ShouldStop()) return 0;
    if (ready==1 && (uint32_t)func_8007F7A8()!=PE_LoadU16(0x800B0DD4u)) {
        func_800719E4(1u);
        if (PE_Port_ShouldStop()) return 0;
    }
    for (unsigned remaining=2000;;) {
        int status=func_8007C484(out,header_out);
        if (PE_Port_ShouldStop()) return 0;
        remaining--;
        if (status==0) break;
        if (!remaining) return 0;
    }
    header=PE_LoadU32(header_out);record=PE_LoadU32(0x801227E4u);
    frame=PE_LoadU32(header+8u);limit=(int16_t)PE_LoadU16(record+8u);
    if (frame>=(uint32_t)(limit-16)) {
        int32_t factor=(int32_t)(14u-(frame+16u-(uint32_t)limit));
        uint32_t product;
        int32_t high,scaled;
        if (factor<0) factor=0;
        product=(uint32_t)PE_LoadU8(0x800B0DBEu)*(uint32_t)factor;
        high=(int32_t)(((int64_t)(int32_t)product*(int32_t)0x92492493u)>>32);
        scaled=(int32_t)((uint32_t)high+product);
        func_800870F0((uint32_t)((scaled>>3)-((int32_t)product>>31)));
        if (PE_Port_ShouldStop()) return 0;
    }
    header=PE_LoadU32(header_out);frame=PE_LoadU32(header+8u);
    if (frame<(uint32_t)(int32_t)(int16_t)PE_LoadU16(0x801227E8u) ||
        frame>=(uint32_t)(int32_t)(int16_t)PE_LoadU16(PE_LoadU32(0x801227E4u)+8u))
        PE_StoreU8(0x801223F5u,1u);
    header=PE_LoadU32(header_out);
    PE_StoreU16(0x801227E8u,(uint16_t)PE_LoadU32(header+8u));
    if ((int16_t)PE_LoadU16(0x80122418u)!=(int32_t)PE_LoadU16(header+16u) ||
        (int16_t)PE_LoadU16(0x8012241Au)!=(int32_t)PE_LoadU16(header+18u)) {
        RECT rect={0,0,PE_LoadU8(0x800B0DBBu)?480:320,480};
        func_80074F44(&rect,0,0,0);
        if (PE_Port_ShouldStop()) return 0;
        header=PE_LoadU32(header_out);
        PE_StoreU16(0x80122418u,PE_LoadU16(header+16u));
        PE_StoreU16(0x8012241Au,PE_LoadU16(header+18u));
    }
    if (PE_LoadU8(0x800B0DBBu)) {
        uint32_t width=(uint32_t)(int32_t)(int16_t)PE_LoadU16(0x80122418u)*3u;
        width=(width+(width>>31))>>1;
        PE_StoreU16(state+34u,(uint16_t)width);PE_StoreU16(state+26u,(uint16_t)width);
    } else {
        uint16_t width=PE_LoadU16(0x80122418u);
        PE_StoreU16(state+34u,width);PE_StoreU16(state+26u,width);
    }
    PE_StoreU16(state+36u,PE_LoadU16(0x8012241Au));
    PE_StoreU16(state+28u,PE_LoadU16(0x8012241Au));
    PE_StoreU16(state+46u,PE_LoadU16(0x8012241Au));
    return (int32_t)PE_LoadU32(out);
}

/* 122040..122354: decode current frame, prepare next frame, then wait for
 * slice completion. Host query ticks stand in for asynchronous CPU work. */
int func_80122040(void)
{
    if(PE_Port_ShouldStop()) return 0;
    if(PE_LoadU8(0x800B0DBAu)<2u) return 0;
    if(PE_LoadU8(0x801223F8u)==2u) {
        func_80121004((int8_t)(PE_LoadU32(0x8009CDDCu)^1u),(int8_t)PE_LoadU8(0x800B0DBBu));
        if(PE_Port_ShouldStop()) return 0;
        PE_StoreU8(0x801223F8u,0u);
    }
    PE_StoreU32(0x80122414u,PE_LoadU32(0x801223FCu));
    func_8010BFA0(PE_LoadU32(0x801228CCu+PE_LoadU8(0x801228D4u)*4u),PE_LoadU8(0x801223F6u));
    if(PE_Port_ShouldStop()) return 0;
    int32_t words=((int32_t)(int16_t)PE_LoadU16(0x801228F8u)*(int16_t)PE_LoadU16(0x801228FAu))/2;
    func_8010C01C(PE_LoadU32(0x801228D8u+PE_LoadU8(0x801228E0u)*4u),(uint32_t)words);
    if(PE_Port_ShouldStop()) return 0;
    pe_addr_t next=0u;
    for(;;) {
        for(unsigned remaining=2000u;remaining;remaining--) {
            next=(pe_addr_t)func_80121270(0x801228CCu);
            if(PE_Port_ShouldStop()) return 0;
            if(next) break;
            HostFB_VSync(-1);
            if(PE_Port_ShouldStop()) return 0;
        }
        if(next) break;
        (void)func_8007C2A0(0x80122414u);
        for(unsigned attempts=0;;attempts++) {
            if(PE_Port_ShouldStop()) return 0;
            int ready=func_8007F72C();
            if(PE_Port_ShouldStop()) return 0;
            int depth=ready==1?func_8007F778():1;
            if(PE_Port_ShouldStop()) return 0;
            if(ready==1 && depth==0) {
                (void)func_80080D5C(2,0x80122414u,0x801FFE70u);
                if(PE_Port_ShouldStop()) return 0;
                int started=func_80081314(0x80122414u,0x1E0u);
                if(PE_Port_ShouldStop()) return 0;
                if(started) break;
            }
            HostFB_VSync(-1);
            if(PE_Port_ShouldStop()) return 0;
            if(!PE_CdReg_DeviceEnabled() || attempts==0x100000u) {
                Bootstrap_ReturnVoid("movie_retry_wait","func_80122040");
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
            }
        }
    }
    uint32_t table=PE_LoadU32(0x80122430u);
    uint16_t count=PE_LoadU16(0x800B0DBCu);
    uint32_t bank=PE_LoadU8(0x801228D4u)^1u;
    PE_StoreU8(0x801228D4u,(uint8_t)bank);PE_StoreU16(0x800B0DBCu,(uint16_t)(count+1u));
    /* A nonzero slot has not been proven complete for every producer/IRQ
     * ordering. Keep that evidence gap separate from this existing path. */
    (void)func_8010C89C(next,PE_LoadU32(0x801228CCu+bank*4u),table,0u);
    if(PE_Port_ShouldStop()) return 0;
    func_8007C394(next);
    uint32_t remaining=0x800000u;
    while(!PE_LoadU8(0x801228FCu)) {
        if(--remaining==0u) {
            PE_StoreU8(0x801228FCu,1u);
            bank=PE_LoadU8(0x801228F2u)^1u;PE_StoreU8(0x801228F2u,(uint8_t)bank);
            PE_StoreU16(0x801228F4u,PE_LoadU16(0x801228E2u+bank*8u));
            PE_StoreU16(0x801228F6u,PE_LoadU16(0x801228E4u+bank*8u));
        }
        if(PE_LoadU8(0x801228FCu)) break;
        HostFB_VSync(-1);
        if(PE_Port_ShouldStop()) return 0;
    }
    uint8_t end=PE_LoadU8(0x801223F5u);
    PE_StoreU8(0x801228FCu,0u);
    if(end!=1u) return 1;
    PE_StoreU8(0x800B0DBAu,(uint8_t)(PE_LoadU8(0x800B0DBAu)-1u));
    func_8010C0D8(0u);
    if(PE_Port_ShouldStop()) return 0;
    func_8007A2A4();
    if(PE_Port_ShouldStop()) return 0;
    (void)func_80080DC4(9,0u,0u);return 0;
}
void func_80122354(void)
{
    PE_StoreU8(0x800B0DBAu,(uint8_t)(PE_LoadU8(0x800B0DBAu)-1u));
    func_800870F0(0u);
    if(PE_Port_ShouldStop()) return;
    func_8010C0D8(0u);
    if(PE_Port_ShouldStop()) return;
    func_8007A2A4();
    if(PE_Port_ShouldStop()) return;
    (void)func_80080DC4(9,0u,0u);
}
/* 121C04..122040: movie player. Selects the movie record by id, configures
 * both display environments, builds and searches the STR filename, seeds
 * the slice geometry from the record and the display bank, initializes the
 * libpress tables and the 1214D4 output callback, opens the record pool
 * and the streaming read, then decodes the first frame. The readiness
 * wait and the Setloc/ReadS retry need the enabled CD device; a disabled
 * device keeps the recorded boundaries visible (80D5C returns 0 for the
 * non-null response accommodation, the decode retry stops at
 * movie_retry_wait through the shared 122040 boundary text). Returns 0. */
int func_80121C04(int id)
{
    unsigned selector=(uint32_t)(uint16_t)id;
    pe_addr_t record;
    char name[64];
    int32_t search;
    uint32_t bank_word,x,start,end;
    if(selector>=0x2Fu) return 0;
    if(PE_Port_ShouldStop()) return 0;
    PE_StoreU8(0x800B0DBFu,(uint8_t)id);
    record=0x80122438u+selector*20u;
    PE_StoreU32(0x801227E4u,record);
    PE_StoreU8(0x800B0DBBu,PE_LoadU8(record+4u));
    func_80121004(0,(int8_t)PE_LoadU8(0x800B0DBBu));
    if(PE_Port_ShouldStop()) return 0;
    func_80121004(1,(int8_t)PE_LoadU8(0x800B0DBBu));
    if(PE_Port_ShouldStop()) return 0;
    name[0]=0;
    /* 0x80121C9C..CB4: a1 = &"\FMV1" / &"\FMV2" (addiu, not a load). */
    strcat(name,(const char *)PE_TranslateConst(selector<0x15u?
        0x80120FF4u:0x80120FFCu,8));
    strcat(name,(const char *)PE_TranslateConst(PE_LoadU32(record),16));
    search=0;
    for(unsigned cycles=0;;) {
        int ready=func_8007F72C();
        if(PE_Port_ShouldStop()) return 0;
        int depth=ready==1?func_8007F778():1;
        if(PE_Port_ShouldStop()) return 0;
        if(ready==1 && depth==0) {
            search=func_80081414(0x801223FCu,name);
            if(PE_Port_ShouldStop()) return 0;
            if(search!=0 && search!=-1) break;
        }
        HostFB_VSync(-1);
        if(PE_Port_ShouldStop()) return 0;
        if(!PE_CdReg_DeviceEnabled() || ++cycles==0x100u) {
            Bootstrap_ReturnVoid("movie_player_search_wait","func_80121C04");
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
        }
    }
    PE_StoreU32(0x80122414u,PE_LoadU32(0x801223FCu));
    if(getenv("PE_FMV_LOG"))
        fprintf(stderr,"[FMV] movie id=%u name=%s rec: +4=%02X +6(end)=%d +8=%04X +A=%04X +C=%04X +E=%04X +10=%04X\n",
                selector,name,PE_LoadU8(record+4u),(int)(int16_t)PE_LoadU16(record+6u),
                PE_LoadU16(record+8u),PE_LoadU16(record+0xAu),PE_LoadU16(record+0xCu),
                PE_LoadU16(record+0xEu),PE_LoadU16(record+0x10u));
    bank_word=PE_LoadU32(0x8009CDDCu);
    x=PE_LoadU16(record+0xAu);
    start=PE_LoadU16(record+0xCu);
    end=(uint32_t)(int32_t)(int16_t)PE_LoadU16(record+6u);
    PE_StoreU8(0x801228F2u,(uint8_t)bank_word);
    PE_StoreU32(0x801228CCu,PE_LoadU32(0x80122420u));
    PE_StoreU32(0x801228D0u,PE_LoadU32(0x80122424u));
    PE_StoreU32(0x801228D8u,PE_LoadU32(0x80122428u));
    PE_StoreU32(0x801228DCu,PE_LoadU32(0x8012242Cu));
    PE_StoreU8(0x801228D4u,0u);
    PE_StoreU8(0x801228E0u,0u);
    PE_StoreU16(0x801228E2u,(uint16_t)x);
    PE_StoreU16(0x801228EAu,(uint16_t)x);
    PE_StoreU16(0x801228E4u,(uint16_t)(start+0xF0u));
    PE_StoreU16(0x801228ECu,(uint16_t)start);
    PE_StoreU16(0x801228F4u,PE_LoadU16(0x801228CCu+(bank_word&0xFFu)*8u+0x16u));
    PE_StoreU16(0x801228F6u,PE_LoadU16(0x801228CCu+(bank_word&0xFFu)*8u+0x18u));
    PE_StoreU16(0x801228F8u,PE_LoadU8(0x800B0DBBu)?0x18u:0x10u);
    PE_StoreU8(0x801228FCu,0u);
    func_8010BE3C(0);
    if(PE_Port_ShouldStop()) return 0;
    func_8010C0D8(0x801214D4u);
    if(PE_Port_ShouldStop()) return 0;
    func_8007A214(PE_LoadU32(0x80122434u),0x40u);
    if(PE_Port_ShouldStop()) return 0;
    func_8007C304(1u,(int32_t)end,-1,0u,0u);
    if(PE_Port_ShouldStop()) return 0;
    for(unsigned cycles=0;;) {
        int ready=func_8007F72C();
        if(PE_Port_ShouldStop()) return 0;
        int depth=ready==1?func_8007F778():1;
        if(PE_Port_ShouldStop()) return 0;
        if(ready==1 && depth==0) {
            (void)func_80080D5C(2,0x801223FCu,0x801FFE90u);
            if(PE_Port_ShouldStop()) return 0;
            int started=func_80081314(0x801223FCu,0x1E0u);
            if(PE_Port_ShouldStop()) return 0;
            if(started) break;
        }
        HostFB_StreamTick();
        if(PE_Port_ShouldStop()) return 0;
        if(!PE_CdReg_DeviceEnabled() || ++cycles==0x100u) {
            Bootstrap_ReturnVoid("movie_player_start_wait","func_80121C04");
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
        }
    }
    func_800870F0(PE_LoadU8(0x800B0DBEu));
    if(PE_Port_ShouldStop()) return 0;
    func_8010BD4C(PE_LoadU32(0x80122430u),0u);
    if(PE_Port_ShouldStop()) return 0;
    {
        pe_addr_t arena=0x801228CCu,table=PE_LoadU32(0x80122430u),next=0u;
        unsigned cycles=0u;
        for(;;) {
            /* Retail polls these 2000 iterations while the 2x drive keeps
             * streaming in real time.  The guest's poll batch stands for far
             * more CPU time than a 1024-cycle counter query, so advance the
             * modeled device with the stream-wait quantum; otherwise only
             * ~3 sectors arrive inside the retry budget and the player
             * restarts mid-frame from sector 0. */
            for(unsigned attempts=0x7D0u;attempts;attempts--) {
                next=(pe_addr_t)func_80121270(arena);
                if(PE_Port_ShouldStop()) return 0;
                if(next) break;
                HostFB_StreamTick();
                if(PE_Port_ShouldStop()) return 0;
            }
            if(next) {
                uint32_t flip=PE_LoadU8(0x801228D4u)^1u;
                uint16_t count=PE_LoadU16(0x800B0DBCu);
                PE_StoreU8(0x801228D4u,(uint8_t)flip);
                PE_StoreU16(0x800B0DBCu,(uint16_t)(count+1u));
                /* A nonzero slot alone is not proof of a complete frame. */
                (void)func_8010C89C(next,PE_LoadU32(arena+flip*4u),table,0u);
                if(PE_Port_ShouldStop()) return 0;
                func_8007C394(next);
                PE_StoreU8(0x801223F5u,0u);
                PE_StoreU8(0x800B0DBAu,(uint8_t)(PE_LoadU8(0x800B0DBAu)+1u));
                PE_StoreU16(0x800B0DBCu,1u);
                return 0;
            }
            /* Retail repeats the Setloc/ReadS retry until a slot arrives.
             * The bounded host retry remains an explicit boundary; nominal
             * device timing does not prove completeness for every ordering. */
            if(PE_CdReg_DeviceEnabled() && ++cycles==0x100u) {
                Bootstrap_ReturnVoid("movie_retry_wait","func_80121C04");
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                return 0;
            }
            PE_StoreU32(0x80122414u,PE_LoadU32(0x801223FCu));
            for(;;) {
                if(PE_Port_ShouldStop()) return 0;
                int ready=func_8007F72C();
                if(PE_Port_ShouldStop()) return 0;
                int depth=ready==1?func_8007F778():1;
                if(PE_Port_ShouldStop()) return 0;
                if(ready==1 && depth==0) {
                    (void)func_80080D5C(2,0x80122414u,0x801FFE90u);
                    if(PE_Port_ShouldStop()) return 0;
                    int started=func_80081314(0x80122414u,0x1E0u);
                    if(PE_Port_ShouldStop()) return 0;
                    if(started) break;
                }
                HostFB_StreamTick();
                if(PE_Port_ShouldStop()) return 0;
                if(!PE_CdReg_DeviceEnabled()) {
                    Bootstrap_ReturnVoid("movie_retry_wait","func_80121C04");
                    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                    return 0;
                }
            }
        }
    }
}

/* 6E60C..6E6A8: restore field presentation after the movie display. */
void func_8006E60C(void)
{
    if(!(PE_LoadU32(0x800B0CD8u)&0x08000000u)) return;
    RECT rect={0,0,320,448};func_80074F44(&rect,0,0,1);
    if(PE_Port_ShouldStop()) return;
    func_80074DC0(0);
    if(PE_Port_ShouldStop()) return;
    (void)func_80068B94();
    if(PE_Port_ShouldStop()) return;
    uint8_t flags=PE_LoadU8(0x800B0CE6u);
    uint32_t display=PE_LoadU32(0x800B0CD8u);
    PE_StoreU8(0x800B0CE6u,flags|2u);PE_StoreU32(0x800B0CD8u,display&0xF7FFFDFFu);
}
