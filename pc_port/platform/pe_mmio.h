/*
 * Memory-mapped I/O dispatch for guest accesses to modelled hardware
 * registers.  ONE table (pe_mmio.c, `g_pe_mmio_regs`) lists every modelled
 * register; any other I/O address stays a loud abort in PE_Translate.
 *
 *   0x1F801040  SIO0 DATA    8  pe_sio0   read (pops RX FIFO) / write (TX)
 *   0x1F801044  SIO0 STAT   16  pe_sio0   read only
 *   0x1F801048  SIO0 MODE   16  pe_sio0   read / write
 *   0x1F80104A  SIO0 CTRL   16  pe_sio0   read / write
 *   0x1F80104E  SIO0 BAUD   16  pe_sio0   read / write
 *   (SIO0 0x1F801041..43 preview, 0x1F80104C MISC: NOT modelled -> abort.)
 *   0x1F801070  I_STAT      16  pe_irq    read / write (W0C)
 *   0x1F801074  I_MASK      16  pe_irq    read / write
 *   0x1F8010F0  DPCR        32  pe_gpu    read / write
 *   0x1F8010F4  DICR        32  pe_gpu    read / write
 *   0x1F801110  RCNT1 count 16  pe_timer1 read only
 *   0x1F801114  RCNT1 mode  16  pe_timer1 read; write only the retail 0x107
 *   0x1F801120  RCNT2 count 16  pe_rcnt2  read / write
 *   0x1F801124  RCNT2 mode  16  pe_rcnt2  read (clears reached) / write
 *   0x1F801128  RCNT2 target16  pe_rcnt2  read / write
 *   (RCNT0 0x1F801100..0x1F80110B, RCNT1 target: NOT modelled -> abort.)
 *   0x1F801C00..0x1F801DFF  SPU registers (pe_spu_dma.h PE_SpuRegister_*):
 *               halfword / aligned word / byte read, halfword / aligned word
 *               write.  Its own host-pointer shadow block: every changed
 *               halfword is committed in address order; KON/KOFF
 *               (0x1F801D88..0x1F801D8F) refresh as 0 so a repeated key-on
 *               mask is still written.
 * Register semantics: https://psx-spx.consoledev.net/timers/ and
 * /interrupts/ and /dmachannels/.  KSEG0/KSEG1 aliases (0x9F80xxxx,
 * 0xBF80xxxx) resolve to the same registers.
 *
 * Two access paths:
 *   * PE_Load/PE_Store (the value API) call PE_MMIO_Load/PE_MMIO_Store:
 *     exact, in program order.  A 32-bit access to a 16-bit register
 *     reads 0 in the upper half and ignores it on write.
 *   * PE_Translate (a host pointer, as generated TUs use through
 *     PE_DECOMP_PTRGLOBAL) returns a SHADOW of the modelled block
 *     (0x1F801040..4F, 0x1F801070..77, 0x1F8010F0..F7, 0x1F801100..2F): refreshed from the
 *     device on every translate, and host-pointer writes are committed to
 *     the device on the next translate, PE_Load/Store of MMIO, or device
 *     tick (PE_MMIO_Commit).  Limits of the shadow path, documented:
 *     a write equal to the last refreshed value is not seen (so a
 *     same-value RCNT mode write does not reset the counter), several
 *     writes between commits are applied in table order, reads through
 *     the pointer do not clear RCNT reached flags, and a pointer READ of an
 *     unmodelled slot inside a block returns the poison byte 0xDE (a pointer
 *     WRITE to one aborts at commit).
 */
#ifndef PE_MMIO_H
#define PE_MMIO_H
#include <stddef.h>
#include <stdint.h>
#include "pe_guest_ram.h"

int PE_MMIO_IsModelled(pe_addr_t address, size_t size);   /* value path */
int PE_MMIO_InShadowBlock(pe_addr_t address, size_t size);
uint32_t PE_MMIO_Load(pe_addr_t address, size_t size);
void PE_MMIO_Store(pe_addr_t address, size_t size, uint32_t value);
void *PE_MMIO_Translate(pe_addr_t address, size_t size);
void PE_MMIO_Commit(void);
void PE_MMIO_Reset(void);

#endif
