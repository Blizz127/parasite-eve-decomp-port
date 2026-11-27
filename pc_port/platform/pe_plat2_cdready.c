/* libcd data-ready callback (retail 0x8007C214, bound as func_8007C214 / pe_plat_cd_data_ready_callback in
 * pe_plat/cd.h): publish the sector record the CD DMA has just filled, advance the record index and run the
 * chained callback.
 *
 * PROVENANCE.  Written clean-room from the black-box vector set cdgen61 (ready.txt, 800 cases; batch
 * cdgen52-61, docs/evidence/native-cdx-20261126/cdgen61/REPORT.md), the checker
 * pc_port/tools/pe_cdx_cdgen61_check.c on pe_cdx_common_canary.h, the public headers (pe_plat/cd.h,
 * pe_guestcode.h, pe_guest_ram.h, bootstrap/game_port.h), the suite tests that exercise the entry
 * (tests/test_native.c CDQ1_7C214_*, tests/test_hook_registry.h, tests/test_worker_registry.h) and psx-spx.
 * NOT read: the previous provider file, src/, asm/, vendor/, game/decomp_hand/, game/boot bodies,
 * pe_stream.c, *_retail.c, oracle sources, retail_*_cases.h.  Disclosed exposures: a grep printed the previous
 * provider's definition line (return type and name only) and two lines of pe_libcd.c (a comment and a
 * stop-guarded CD flush call in another function); neither concerns this entry.  The behaviour below is what the
 * vectors and the suite tests show; no claim is made about the original's source.  Derivation and the
 * unsampled arms: docs/evidence/native-cdx-20261126/cdrw-a/AUTHOR-REPORT.md.
 *
 * Guest words (little-endian):
 *   0x800BE9E4  current record index `fi` (any 32-bit value; the record address wraps mod 2^32)
 *   0x800C0DC8  record base pointer; record `fi` lives at base + (fi << 5), 32 bytes per record
 *   0x800BE998  next record index
 *   0x800B0CC8  chained callback address (0 = none)
 *   0x800A3490 / 0x800A3494  the published sector word / record word
 *   0x800B89F4  active flag, cleared when the sequence completes
 *
 * Behaviour (vectors, all 800 cases the same shape):
 *   e = base + (fi << 5);  sector = word(e + 0x1C);  aux = word(e + 8)
 *   halfword(e) = 2;  word(0x800A3490) = sector;  word(0x800A3494) = aux;  word(0x800BE9E4) = word(0x800BE998)
 *   chain = word(0x800B0CC8); a nonzero chain is called with three argument registers
 *   (a0 = the chain address itself, a1 = 0, a2 = 0x800A3490), the result ignored
 *   word(0x800B89F4) = 0
 * The five stores above the call all precede it; the 0x800B89F4 clear follows it.
 * Not sampled: a chain value other than 0 or the three vector sentinels is called the same way; the original's
 * read of the old word at 0x800A3490 has no observable effect and is not reproduced.
 *
 * Host contract (port glue, from tests/test_hook_registry.h): the chain runs through the guest-hook seam
 * PE_GuestHook_Call (slot "func_8007C214_B0CC8_chain"), which refuses an address with no native code
 * (unresolved-boundary stop); a stop after the callback leaves the active flag untouched. */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_plat/cd.h"
#include "game_port.h"

#define W_FRAME_INDEX  0x800BE9E4u
#define W_RECORD_BASE  0x800C0DC8u
#define W_NEXT_INDEX   0x800BE998u
#define W_CHAIN        0x800B0CC8u
#define W_SECTOR       0x800A3490u
#define W_RECORD       0x800A3494u
#define W_ACTIVE       0x800B89F4u

void pe_plat_cd_data_ready_callback(void)
{
    uint32_t fi = PE_LoadU32(W_FRAME_INDEX);
    pe_addr_t e = PE_LoadU32(W_RECORD_BASE) + (fi << 5);
    uint32_t sector = PE_LoadU32(e + 0x1Cu);
    uint32_t aux = PE_LoadU32(e + 8u);
    uint32_t chain;
    int ignored = 0;

    PE_StoreU16(e, 2u);
    PE_StoreU32(W_SECTOR, sector);
    PE_StoreU32(W_RECORD, aux);
    PE_StoreU32(W_FRAME_INDEX, PE_LoadU32(W_NEXT_INDEX));
    chain = PE_LoadU32(W_CHAIN);
    if (chain != 0u) {
        (void)PE_GuestHook_Call("func_8007C214_B0CC8_chain", "func_8007C214", chain, 3u,
                                chain, 0u, W_SECTOR, 0u, &ignored);
        if (PE_Port_ShouldStop())
            return;
    }
    PE_StoreU32(W_ACTIVE, 0u);
}
