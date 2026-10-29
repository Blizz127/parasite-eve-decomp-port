/*
 * Memory-mapped I/O dispatch for guest accesses to modelled hardware
 * registers. pe_mmio.c owns register dispatch and dedicated device ranges; any other I/O address stays a loud abort in PE_Translate.
 *
 *   0x1F801040  SIO0 DATA 8/16/32 pe_sio0 value read / low-byte TX write
 *               16-bit read previews two/pops one; 32-bit read pops four.
 *               Legacy pointer shadow still covers only the byte register.
 *   0x1F801044  SIO0 STAT   16  pe_sio0   read only
 *   0x1F801048  SIO0 MODE   16  pe_sio0   read / write
 *   0x1F80104A  SIO0 CTRL   16  pe_sio0   read / write
 *   0x1F80104E  SIO0 BAUD   16  pe_sio0   read / write
 *   (SIO0 0x1F801041..43 preview, 0x1F80104C MISC: NOT modelled -> abort.)
 *   0x1F801070  I_STAT      16  pe_irq    read / write (W0C)
 *   0x1F801074  I_MASK      16  pe_irq    read / write
 *   0x1F801080..0x1F801088 MDEC DMA0 MADR/BCR/CHCR: aligned word value
 *               reads / writes; forward request-mode input, no pointer alias.
 *   0x1F801090..0x1F801098 MDEC DMA1 MADR/BCR/CHCR: aligned word value
 *               reads / writes; request-mode output, whole color macroblocks
 *               or monochrome chunks, no pointer alias.
 *   0x1F8010A0..0x1F8010A8 GPU DMA2 MADR/BCR/CHCR: aligned word value
 *               reads / writes; image/command request mode and RAM->GP0 lists.
 *               Command blocks/list nodes retain parser state between services.
 *               Live DPCR/GP1/DREQ gates service; readback requires C0 setup.
 *               No mutable pointer alias or subword access.
 *   0x1F8010B0..0x1F8010B8 CD DMA3 MADR/BCR/CHCR: aligned word value
 *               reads / writes through pe_cdreg (including its service gates).
 *               No mutable pointer alias or subword access.
 *   0x1F8010E0..0x1F8010E8 DMA6 OTC MADR/BCR/CHCR: aligned word value
 *               access, native synchronous reverse ordering-table clear.
 *   DMA0..3/6 CHCR at +0xC mirrors +8 (aligned word value access).
 *               DMA4/5 remain unmodelled; no second register storage.
 *   0x1F8010F0  DPCR        32  pe_gpu    read / write
 *   0x1F8010F4  DICR        32  pe_gpu    read / write
 *   0x1F801110  RCNT1 count 16  pe_timer1 read / write
 *   0x1F801114  RCNT1 mode  16  pe_timer1 read/write (HBlank, all sync modes, no IRQ)
 *   0x1F801118  RCNT1 target16  pe_timer1 read / write
 *   0x1F801120  RCNT2 count 16  pe_rcnt2  read / write
 *   0x1F801124  RCNT2 mode  16  pe_rcnt2  read (clears reached) / write
 *   0x1F801128  RCNT2 target16  pe_rcnt2  read / write
 *   (RCNT0 0x1F801100..0x1F80110B: NOT modelled -> abort.)
 *   0x1F801800..0x1F801803 CD registers: byte value reads / writes;
 *               RDDATA at 0x1F801802 also permits a halfword read (two FIFO
 *               bytes, low first). No CD host-pointer shadow or word MMIO.
 *   0x1F801810/1814 GPU data/control: aligned word value reads / writes
 *               through pe_gpu. FIFO reads consume data; status reads do not
 *               advance it. No mutable pointer alias or subword access.
 *   0x1F801820/1824 MDEC command/control: aligned word writes through pe_mdec.
 *               Quant/scale/decode input and reset/control. Complete received
 *               macroblocks can decode early. Data reads consume four bytes in raw
 *               8x8 block order; status reads, subword accesses and mutable
 *               pointers remain unsupported.
 *   0x1F801C00..0x1F801DFF  SPU registers (pe_spu_dma.h PE_SpuRegister_*):
 *               halfword / aligned word / byte read, halfword / aligned word
 *               write.  Its own host-pointer shadow block: every changed
 *               halfword is committed in address order; KON/KOFF
 *               (0x1F801D88..0x1F801D8F) refresh as 0 so a repeated key-on
 *               mask is still written.
 *   0x1F801E00..0x1F801E5F  SPU current voice volumes (pe_spu):
 *               byte / aligned halfword / aligned word value reads only;
 *               signed live sweep levels, zero when synthesis is inactive.
 *               Writes and host-pointer translation remain unsupported.
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
