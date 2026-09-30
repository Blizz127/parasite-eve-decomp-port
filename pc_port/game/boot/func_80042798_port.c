/*
 * Phase 6E-EV1 — func_80042798 event-record cleanup walk, translated retail.
 *
 * Retail: [0x80042798,0x80042848), 44 words, asm/disc1/307CC.s:2818-2869.
 * No matching src/ C (this rung is a native translation, not a decomp
 * leaf; see the OTC1 note in docs/evidence/pe-otc1-clearotagr/REPORT.md
 * for the standard this port is held to).
 *
 * Body, ROM order:
 *   prologue (sp-0x30, save ra/s5/s4/s3/s2/s1, s0 in the beqz slot);
 *   v1 = &D_800A0ED4; statically-true unsigned range guard
 *   (v1 < v1+0x830: no wrap for a fixed symbol, so the beqz arm is dead);
 *   s5 = 8, s4 = 10, s3 = -1, s2 = 12; cursor s0 = v1+1,
 *   bound s1 = v1+0x831;
 *   loop: tag = lbu(cursor); nop; beq tag,8 -> hit; nop;
 *         bne tag,10 -> miss; nop;
 *   hit: a0 = lw(cursor+0xB); jal func_80072774; nop;
 *        sw -1,(cursor+0xB); sb 12,(cursor);
 *   miss: cursor += 0x418; sltu against bound; bnez -> loop; nop.
 *   epilogue; jr ra.
 *
 * The +0xB word lands at record+0xC (base 0x800A0ED4 + 0xC + 0x418*i):
 * base and stride are both 4-multiples, so the retail lw/sw need no
 * adaptation.  The [base+1, base+0x831) window holds exactly two records;
 * the port keeps retail's cursor/bound shape, not a baked count.
 *
 * Lifecycle fit: func_80042538 (translated) memsets this 0x830-byte block
 * and writes the two -1 sentinels at +0xC/+0x424 — the exact words this
 * walk consumes.  Tags 8/10 mark records whose payload word is a live
 * handle; after the call the word is invalidated and the tag advances
 * to 12.
 *
 * func_80072774 is a 3-word BIOS-vector trampoline (jr 0xB0, $t1 = 0x36);
 * it remains the named callee boundary, recorded with the retail
 * trampoline identity and the record word — the same edge the 5C1EC zero
 * path used to preserve for all of 42798, now narrowed to the call.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "pe_memcard.h"

#define GA_EV_TABLE_BASE 0x800A0ED4u
#define GA_EV_TABLE_END_OFF 0x831u
#define GA_EV_RANGE_OFF 0x830u
#define GA_EV_STRIDE 0x418u
#define GA_EV_TAG_HIT0 8u
#define GA_EV_TAG_HIT1 10u
#define GA_EV_TAG_DONE 12u
#define GA_EV_CALLEE 0x80072774u

void func_80042798(void)
{
    pe_addr_t base = GA_EV_TABLE_BASE;
    pe_addr_t bound = base + GA_EV_TABLE_END_OFF;
    pe_addr_t cursor;

    /* Statically-true unsigned range guard (retail beqz arm is dead). */
    if (!(base < base + GA_EV_RANGE_OFF))
        return;

    cursor = base + 1u;
    while (cursor < bound) {
        uint32_t tag = PE_LoadU8(cursor);
        if (tag == GA_EV_TAG_HIT0 || tag == GA_EV_TAG_HIT1) {
            uint32_t handle = PE_LoadU32(cursor + 0xBu);
            if (PE_Memcard_BoundaryMode()) {
                /* Test frontier (see pe_memcard.h). */
                (void)Bootstrap_ReturnInt4Indirect(
                    "func_80072774", "func_80042798", 0, GA_EV_CALLEE,
                    handle, 0u, 0u, 0u, NULL, 0u);
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                return;
            }
            (void)func_80072774((int)handle);   /* BIOS B(36h) close */
            PE_StoreU32(cursor + 0xBu, 0xFFFFFFFFu);
            PE_StoreU8(cursor, GA_EV_TAG_DONE);
        }
        cursor += GA_EV_STRIDE;
    }
}
