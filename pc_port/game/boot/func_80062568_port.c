/*
 * Phase 6E-B20 — func_80062568: free-list pool initializer.
 *
 * Raw body: 20 words / 0x50, exe 0x80062568–0x800625B7, file 0x52D68,
 * live split asm/disc1/52ABC.s:206–229; all 20 instruction words
 * verified exact against the SHA-exact retail executable.
 *
 * Sole call site: func_800527C8 @0x80052804 (nop delay slot, no args,
 * return ignored).  Now the B19 strict frontier.
 *
 * Operation (ROM order):
 *   1. Base = D_800A22E0 (guest address), end = base + 0xD80 (0x800A3060).
 *   2. Free-list link loop: for each slot from base to base+0xD80
 *      step 0x90 (24 slots of 144 bytes each):
 *        slot[0] = address_of_next_slot
 *        advance to next slot
 *      Last slot at D_800A2FD0 gets the sentinel 0x800A3060.
 *   3. sw 0 → D_800A2FD0 (null-terminate the free list).
 *   4. sw D_800A22E0 → D_8009D158 ($gp + 0x3E8) — free-list head.
 *   5. sw 0 → D_8009D15C ($gp + 0x3EC) — tail/counter.
 *   6. sw 0 → D_8009D154 ($gp + 0x3E4) — auxiliary field.
 *   7. jr $ra; nop.
 *
 * No callees.  No SDK, GPU, MDEC, audio, disc, or input access.
 * Idempotent; PE_RamReset restores the initial contract.
 *
 * void func_80062568(void).
 *
 * Classification: 1 — translated retail logic.
 */
#include "psx_compat.h"

#define POOL_BASE       0x800A22E0u
#define POOL_END        0x800A3060u
#define POOL_STRIDE     0x90u
#define GA_POOL_HEAD    (0x8009CD70u + 0x3E8u)  /* D_8009D158 */
#define GA_POOL_TAIL    (0x8009CD70u + 0x3ECu)  /* D_8009D15C */
#define GA_POOL_AUX     (0x8009CD70u + 0x3E4u)  /* D_8009D154 */

void func_80062568(void)
{
    pe_addr_t slot;

    /* Build the free-list chain.  Each slot stores a pointer to the
     * next slot; the last slot is NULL-terminated afterward. */
    for (slot = POOL_BASE; slot < POOL_END; slot += POOL_STRIDE) {
        PE_StoreU32(slot, (uint32_t)(slot + POOL_STRIDE));
    }

    /* Null-terminate the last slot. */
    PE_StoreU32(POOL_END - POOL_STRIDE, 0);

    /* Initialize head, tail, and auxiliary fields. */
    PE_StoreU32(GA_POOL_HEAD, POOL_BASE);
    PE_StoreU32(GA_POOL_TAIL, 0);
    PE_StoreU32(GA_POOL_AUX,  0);
}
