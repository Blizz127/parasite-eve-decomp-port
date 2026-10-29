/*
 * SIO0 — controller / memory-card serial port, 0x1F801040..0x1F80104F.
 *
 * Retail use (pe_dis.sh 0x800830DC 0x1D8, 0x800832B4 0x234; callers in
 * pc_port/game/decomp_hand/absent_cd_port.c, all through the pointer global
 * D_8009B788 = 0x1F801040): DATA byte write/read (sb/lbu +0), STAT halfword
 * read (lhu +4: bits 0, 1, 9), MODE write (+8 = 0x0D), CTRL write/read
 * (+0xA: 0x40, 0, 0x1003/0x3003, |= 0x10), BAUD write (+0xE: 0x88/0x22).
 * Those registers plus DATA halfword/word value accesses are modelled
 * (pe_mmio.h table); direct preview-byte offsets remain unsupported.
 *
 * Register semantics, quoted from psx-spx "Serial Interfaces (SIO)"
 * (https://psx-spx.consoledev.net/serialinterfacessio/; source
 * psx-spx.github.io docs/serialinterfacessio.md):
 *   SIO_DATA 1F801040 "When writing to DR, both SR.0 and SR.2 become zero.
 *     As soon as the transfer starts, SR.0 becomes set ... As soon as the
 *     transfer of the most recently written byte ends, SR.2 becomes set."
 *     "Data can be read from DR when SR.1 is set, that flag gets
 *     automatically cleared after reading from DR (unless there are still
 *     further bytes in the RX FIFO)."
 *   SIO_STAT 1F801044 "0 TXRDY TX FIFO Not Full (1=Ready for new byte)",
 *     "1 RXRDY RX FIFO Not Empty", "2 TXU TX Idle (1=Idle/Finished)",
 *     "7 DSR DSR Input Level" — "On SIO0, DSR is wired to the /ACK pin",
 *     "9 IRQ Interrupt Request (0=None, 1=IRQ) ... (sticky)".
 *   SIO_MODE 1F801048 bits 0..5 and 8; 6..7 are SIO1-only, 9..15 zero.
 *   SIO_CTRL 1F80104A "0 TXEN TX Enable", "1 DTR ... (SIO0) bit 1 will pull
 *     (assert) /CS low", "2 RXEN (SIO0: 0=only receive when /CS low,
 *     1=force receiving single byte)"; RXEN clears after reception.
 *     "4 ERRRST Acknowledge ... Reset
 *     SR.Bits 3,4,5,9 (W)", "6 INTRST Reset ... Reset most registers to
 *     zero (W)", bits "14-15 Not used (always zero)".
 *   SIO_BAUD 1F80104E "Baudrate Reload value".
 *
 * RX backing slots retain received bytes even after RXRDY clears: each
 * receive fills all unused slots, each read shifts eight slots and inserts
 * zero at the tail. Empty reads therefore eventually return zero. Reset
 * clears both count and retained data. A full FIFO overwrites its last entry.
 *
 * Attached device: NONE. Received bytes are FFh (floating High-Z), DSR
 * (STAT.7) stays zero, and the retail /ACK interrupt (CTRL.12) never fires.
 * This is distinct from CTRL.10 TX-ready and CTRL.11 RX-FIFO interrupts:
 * those can latch STAT.9 and I_STAT.7 with no attached device. CTRL.8..9
 * selects the receive threshold of 1/2/4/8 bytes. Acknowledging STAT.9 via
 * CTRL.4 while its condition persists produces a fresh interrupt edge;
 * clearing I_STAT alone does not. No card/controller protocol is modelled.
 *
 * Timing: an enabled byte transfer (8 bit times at the programmed baud)
 * completes before the next guest status poll. A write while TXEN is off
 * occupies the TX holding slot (TXRDY/TXU clear); enabling TXEN completes
 * it, sets TXRDY/TXU and applies the receive gates. Reset cancels the slot.
 * The port has no per-instruction clock, so the
 * transfer duration is not observable here; the retail code polls these
 * flags and never measures the transfer itself (its timeouts use RCNT2).
 * Host-time-independent and deterministic.
 */
#ifndef PE_SIO0_H
#define PE_SIO0_H
#include <stdint.h>

void PE_Sio0_Reset(void);
void PE_Sio0_WriteData(uint8_t value);
uint8_t PE_Sio0_ReadData(void);     /* pops the RX FIFO */
uint8_t PE_Sio0_PeekData(void);     /* no pop (MMIO shadow) */
/* Value-register accesses: 2 bytes previews two/pops one; 4 pops four. */
uint32_t PE_Sio0_ReadDataWide(unsigned bytes); /* only 2 or 4 */
uint16_t PE_Sio0_ReadStat(void);
uint16_t PE_Sio0_ReadMode(void);
void PE_Sio0_WriteMode(uint16_t value);
uint16_t PE_Sio0_ReadCtrl(void);
void PE_Sio0_WriteCtrl(uint16_t value);
uint16_t PE_Sio0_ReadBaud(void);
void PE_Sio0_WriteBaud(uint16_t value);

#endif
