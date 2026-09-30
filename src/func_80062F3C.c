/* Phase 5FX: matching C leaf.
 * VRAM 0x80062F3C / file 0x5373C / size 0x60 (24 words).
 *
 * Scans the node list rooted at D_8009D154 (gp+0x3E4) for the entry with
 * +0x20 == 1 and +0x24 == the argument, then hands that node to func_8006269C.
 * The cursor stays in $a0, so a miss (or an empty list) passes NULL — the port
 * spells the same thing func_8006269C(func_80062A34(1u, id)).
 *
 * The `one` local is load-bearing, not decoration: keeping the loop constant in
 * a variable makes GCC hoist it into the entry block, which is what retail does
 * (`addiu a1,zero,1` before the guard).  Written inline as `== 1`, the constant
 * is materialised inside the loop body instead and the first six words differ
 * (frame adjust sunk below the head load, constant after the branch) while every
 * other word still matched — 6 mismatches.  Swapping the comparison operands
 * (`one == x`) instead flips two `bne` operand orders.
 *
 * D_8009D154 is in the small-data window, so the head load is gp-relative
 * (0x3E4($gp)) — -G8 profile (era_o2_g8).
 */
extern int D_8009D154;

void func_8006269C(int node);

void func_80062F3C(int id) {
    int node = D_8009D154;
    int one = 1;

    while (node != 0) {
        if (*(int *)(node + 0x20) == one && *(int *)(node + 0x24) == id) break;
        node = *(int *)node;
    }
    func_8006269C(node);
}
