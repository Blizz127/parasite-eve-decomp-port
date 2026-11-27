/* CD-ROM controller register window and the host-side access wrappers.
 * Declaration-only header (no function bodies): the port's host model API for the CD-ROM controller registers.
 * Declarations were moved unchanged from the port's own host-model header platform/pe_cdreg.h
 * (commit e15e3ee7c); comments describe hardware behaviour as documented publicly (psx-spx)
 * in the project's own words.  No SDK source, tables or disassembly are reproduced here. */
#ifndef PE_PLAT_CD_REGS_H
#define PE_PLAT_CD_REGS_H

#include <stdint.h>

#include "pe_guest_ram.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CD controller registers + result mailbox. */
#define PE_CDREG_BASE 0x1F801800u
#define PE_CDREG_SIZE 4u
#define PE_CDREG_MAILBOX 0x1F801020u
#define PE_CDREG_MAILBOX_SIZE 4u
/* CD bus timing/control word used by the original stream reader. */
#define PE_CDREG_BUS_CONTROL 0x1F801018u
/* DMA3 registers; enabled-device transfers use the shared DMA IRQ owner. */
#define PE_CDREG_DMA3 0x1F8010B0u

int PE_CdReg_IsMmio(pe_addr_t address, uint32_t size);
void PE_CdReg_Reset(void);
/* Explicit post-BIOS device attachment to the active validated single-track
 * disc. Timing advances only via ServiceDevice; no guest driver state is written here. */
int PE_CdReg_DeviceEnabled(void);
int PE_CdReg_EnableDevice(uint8_t interrupt_mask);
/* Mid-game detach keeps the CD audio mix and ADPMUTE latches. */
void PE_CdReg_DetachDevice(void);
/* At the validated single-track image end, stop and issue INT4 DataEnd.
 * In-range image I/O failures remain unresolved device boundaries. */
void PE_CdReg_ServiceDevice(uint32_t elapsed_cycles);
/* Positive host-time slice ending at the next scheduled CD event. Overdue
 * events blocked on acknowledgment cannot impose a zero-length spin. */
uint32_t PE_CdReg_TimeSlice(uint32_t budget);
void PE_CdReg_ServiceDMA3(void);
typedef struct {
    uint32_t enabled,commands,responses;
    uint32_t next_lba,sectors,data_remaining;
    uint8_t reading;
    uint8_t mode,muted,command_log[16],response_log[16];
    uint8_t motor_off;          /* after Stop (08h), until a spin-up command */
    uint32_t xa_sectors;        /* audio+realtime sectors routed to the SPU */
} PeCdDeviceState;
void PE_CdReg_GetDeviceState(PeCdDeviceState *out);
/* Applied ATV0..3: L->L, L->R, R->R, R->L. Register state, not audio mixing. */
void PE_CdReg_GetAudioVolumes(uint8_t out[4]);
/* ADPCTL bit0 only; command Mute/Demute is separate device state. */
int PE_CdReg_XaMuted(void);
/* Bounded device-response ingress. Rejects pending/unread responses; does
 * not issue a disc command, produce sector data or assert a CPU interrupt. */
int PE_CdReg_PushResponse(uint8_t tag, const uint8_t *bytes, uint32_t count);
uint8_t PE_CdReg_ReadU8(pe_addr_t address);
void PE_CdReg_WriteU8(pe_addr_t address, uint8_t value);
uint32_t PE_CdReg_ReadU32(pe_addr_t address);
void PE_CdReg_WriteU32(pe_addr_t address, uint32_t value);

/* Address-decoding access for table-derived addresses. */
uint8_t PE_CdLoadU8(pe_addr_t address);
void PE_CdStoreU8(pe_addr_t address, uint8_t value);
uint32_t PE_CdLoadU32(pe_addr_t address);
void PE_CdStoreU32(pe_addr_t address, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* PE_PLAT_CD_REGS_H */
