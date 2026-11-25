/* Memory-card BIOS service thunks (kernel A0/B0 vectors).  Written clean-room
 * from the psx-spx kernel documentation ("BIOS Memory Card Functions" and
 * "BIOS Event Summary", https://psx-spx.consoledev.net/kernelbios/, fetched
 * 2026-11-25) and the port's host memory-card model (platform/pe_memcard.c),
 * with no SDK source.
 *
 * The retail addresses are four-word trampolines (`li t2,A0h|B0h; jr t2;
 * li t1,N`), so the contract is the BIOS call.  What psx-spx documents:
 *   A(ABh) _card_info(port)   no return value documented; its subfunction
 *                             B(4Dh) checks whether the last _card_write
 *                             finished correctly;
 *   A(ACh) _card_load(port)   "Invokes asynchronous reading of the memory
 *                             card directory"; no return value documented;
 *   B(4Eh) _card_write(port,sector,src)  asynchronous single-sector write;
 *                             returns 1 = okay, 0 = failed (invalid sector
 *                             number; valid 0..3FFh); a write error is
 *                             reported by the next sector transmission;
 *   B(50h) _new_card()        no arguments/return documented; makes the BIOS
 *                             ignore the card-change flag on the next
 *                             read/write.
 * Events (psx-spx event summary): higher level F4000001h spec 4 = ok, 100h =
 * busy/timeout, 2000h = eject or unformatted, 8000h = write/general error;
 * hardware level F0000011h spec 4 = ok, 100h, 2000h, 8000h, 8001h = errors.
 * psx-spx does NOT say which of these each call raises; the mapping used
 * below (F4000001h for _card_info/_card_load, F0000011h for _card_write) is
 * the host model's, as already implemented in the in-tree pe_bios_card.c
 * definitions these thunks replace.
 *
 * The host model completes the request synchronously: each thunk delivers
 * the event the host card model computes and returns 1.
 *
 * Model limits (carried over unchanged; no new transaction semantics; see
 * docs/evidence/native-card-bios-20261125/REPORT.md for the gap list):
 *  - _card_write does not write a sector and does not validate the sector
 *    number (psx-spx: 0 for an invalid one): the host model has no raw
 *    sector primitive and the only caller (libcard _card_clear, sector 3Fh
 *    from a null source) uses the call to acknowledge a new card.  The
 *    result of PE_Memcard_CardClearResult (clears the port's new-card latch)
 *    is delivered for ANY sector/source; the card image is never modified.
 *    Spec 8000h/8001h are never produced.
 *  - _new_card() is a no-op (the host's new-card latch is only cleared by the
 *    acknowledging write above) and ignores its (host-extra) argument.
 * Bound names: see pe_plat/bios_card.h. */
#include "pe_plat/bios_card.h"
#include "pe_sdk.h"
#include "pe_memcard.h"

/* A(ABh) _card_info */
int pe_plat_bios_card_info(uint32_t chan)
{
    PE_BiosCard_Deliver(0xF4000001u, (uint32_t)PE_Memcard_CardInfoResult(chan));
    return 1;
}

/* A(ACh) _card_load */
int pe_plat_bios_card_load(uint32_t chan)
{
    PE_BiosCard_Deliver(0xF4000001u, (uint32_t)PE_Memcard_CardLoadResult(chan));
    return 1;
}

/* B(4Eh) _card_write(chan, sector, src) */
int pe_plat_bios_card_write(uint32_t chan, int sector, int src)
{
    (void)sector; (void)src;
    PE_BiosCard_Deliver(0xF0000011u, (uint32_t)PE_Memcard_CardClearResult(chan));
    return 1;
}

/* B(50h) _new_card */
void pe_plat_bios_card_new_card(uint32_t chan)
{
    (void)chan;
}
