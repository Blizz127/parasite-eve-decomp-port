/* VRAM 0x8005E788 / file 0x4EF88 / size 0xC8.
 * GP-guarded render-clip setup. When the 0x3B0(gp) subsystem is live, bracket
 * the sequence with func_80073A44 / func_80074DC0 / func_80074A44, resubmit the
 * 0x38C(gp) descriptor and its +0x5C tail, then (when the 0x3C4(gp) object
 * exists) file a 320x204 RECT whose y origin the 0x398(gp) flag selects, and
 * hand the 0x3A8(gp) base + 0xFFF-word tail to func_800753B4. era -O2 -G8.
 *
 * Match notes:
 *   - The second func_80073A44 takes `arg0 == 1 ? 0 : arg0`. Writing it as a
 *     copied local `m` (compared and then reassigned) makes cc1 emit retail's
 *     branch form (`addu $a0,$s0 / bne`) instead of the branchless
 *     xori/sltu/negu/and if-conversion the ternary produces.
 *   - The tail index is a runtime variable pinned to 0xFFF, not a folded
 *     constant: retail materialises 0xFFF once (`$s0`, reorg-hoisted into the
 *     bne delay slot) and strength-reduces to `sll $a0,$s0,2; addu`. It must be
 *     assigned after func_80074DC0 and before `m = arg0` for the allocator to
 *     reuse $s0 and for reorg to pick the bne delay slot.
 *   - The `sh $v0,0x16($sp)` in the func_8007506C jal delay slot is a plain
 *     register-offset store that cc1's own reorg already schedules; it is NOT a
 *     maspsx macro-store case, so no fill gate is involved here.
 */
extern int D_8009D120, D_8009D0FC, D_8009D134, D_8009D108;
extern int *D_8009D118;
typedef struct { short x, y, w, h; } RECT;
int func_80073A44(); int func_80074DC0(); int func_80074A44();
int func_80075424(); int func_800755F0(); int func_8007506C(); int func_800753B4();
void func_8005E788(int arg0) {
    RECT rect; int descriptor; int idx; int m;
    if (D_8009D120 != 0) {
        func_80073A44(1);
        func_80074DC0(0);
        idx = 0xFFF;
        m = arg0;

        if (m == 1) m = 0;

        func_80073A44(m);

        func_80074A44(1);

        descriptor = D_8009D0FC;
        func_80075424(descriptor);
        func_800755F0(D_8009D0FC + 0x5C);
        if (D_8009D134 != 0) {
            rect.x = 0;
            rect.y = D_8009D108 != 0 ? 0xEB : 0xB;
            rect.w = 0x140; rect.h = 0xCC;
            func_8007506C(&rect, D_8009D134);
        }

        func_800753B4(&D_8009D118[idx]);
    }
}
