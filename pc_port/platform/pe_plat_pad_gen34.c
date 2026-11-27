/* libpad PadStartCom (guest 0x80082CF0), written clean-room from black-box
 * vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py cdgen34)
 * and psx-spx, with no SDK source and without reading the previous provider.
 *
 * Observed behaviour: clear the "communication running" word, enter the
 * critical section, make the two kernel interrupt-queue calls (2, 0x800A5AB0) --
 * dequeue (C(03h), 0x8007E1F4) then enqueue (C(02h), 0x8007E1E4), now the
 * host pe_plat_bios_intrp.c chain model -- acknowledge bit 0 of the interrupt-status word the pointer word 0x8009B784
 * names and set bit 0 of the mask word behind it, clear the two RCnt counters
 * (ChangeClearRCnt(3, 0)), leave the critical section, then fire the card slot
 * word 0x8009B728 twice -- with the argument word 0x8009B758, then with that
 * word + 0xF0 -- re-reading both words before each call and stopping with 0
 * when the slot is null.  On success the two counters 0x800A5AC4 / 0x800A5AC0
 * are zeroed, the running word is set and 1 is returned. */
#include <stdint.h>
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_plat/pad.h"
#include "pe_plat/bios_sys.h"
#include "pe_plat/bios_intrp.h"

#define ADDR_RUNNING   0x8009B75Cu
#define ADDR_ISTAT_PTR 0x8009B784u
#define ADDR_ARG       0x8009B758u
#define ADDR_SLOT      0x8009B728u
#define ADDR_CNT_A     0x800A5AC4u
#define ADDR_CNT_B     0x800A5AC0u
#define QUEUE_BLOCK    0x800A5AB0u

#ifndef PAD34_MUTANT
#define PAD34_MUTANT 0
#endif

int pe_plat_pad_start_com(void)
{
    uint32_t p, a, s;
    PE_StoreU32(ADDR_RUNNING, 0u);
    (void)pe_plat_bios_enter_critical();
    (void)pe_plat_bios_sys_deq_int_rp(2u, QUEUE_BLOCK);   /* C(03h) 0x8007E1F4 */
    (void)pe_plat_bios_sys_enq_int_rp(2u, QUEUE_BLOCK);   /* C(02h) 0x8007E1E4 */
    p = PE_LoadU32(ADDR_ISTAT_PTR);
    PE_StoreU32(p, 0xFFFFFFFEu);
    PE_StoreU32(p + 4u, PE_LoadU32(p + 4u) | 1u);
    (void)pe_plat_bios_change_clear_rcnt(3u, 0u);
    pe_plat_bios_exit_critical();
    a = PE_LoadU32(ADDR_ARG);
    s = PE_LoadU32(ADDR_SLOT);
    if (s == 0u) return 0;
    (void)PE_Card_ICall("D_8009B728 (card record reset)", s, 1, a, 0, 0);
    if (PAD34_MUTANT != 1) { a = PE_LoadU32(ADDR_ARG); }   /* mutant 1: stale argument */
    if (PAD34_MUTANT != 2) { s = PE_LoadU32(ADDR_SLOT); }  /* mutant 2: stale slot */
    if (s == 0u) return 0;
    (void)PE_Card_ICall("D_8009B728 (card record reset)", s, 1, a + (PAD34_MUTANT == 3 ? 0xF1u : 0xF0u), 0, 0);
    PE_StoreU32(ADDR_CNT_A, 0u);
    PE_StoreU32(ADDR_CNT_B, 0u);
    PE_StoreU32(ADDR_RUNNING, 1u);
    return 1;
}
