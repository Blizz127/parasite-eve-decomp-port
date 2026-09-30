/*
 * func_80066BD8 — scrolling-backdrop window setup (retail 0x80066BD8).
 *
 * VRAM 0x80066BD8 / file 0x573D8 / size 0xA4 (41 words), inside 56442.s
 * region of the 56438 unit.
 *
 * Snapshots the three old window words D_800BCFE8/EA/EC, writes the new
 * window (arg1..arg3), mode byte D_800BCFEE = 2, the signed halfword
 * D_800BCFF6 = arg0 and the zero word D_800BCFF8, then restores the old
 * words into the D_800BCFF0/2/4 copies. Finally D_800BCFEF becomes arg4's
 * low byte when `(arg4 & 0xFFFF) < 4 && >= 0`, else 1. Returns 0.
 *
 * era -O2 -G0. Levers:
 *   - arg4 is `unsigned short` (retail `lhu $t2,0x10($sp)`), while the
 *     compared value is an `int` local `v = arg4 & 0xFFFF` so cc1 emits
 *     `slti` (signed) plus the (dead but retail-present) `bltz`.
 *   - the success path must `return 0` inside the nested `if`; writing
 *     `if (v < 4 && v >= 0) ... else ...` makes cc1 drop the `bltz`
 *     (10 diffs), and sharing a duplicated else block points the `bltz`
 *     at the epilogue instead of the store-1 block (1 diff).
 */
extern unsigned short D_800BCFE8;
extern unsigned short D_800BCFEA;
extern unsigned short D_800BCFEC;
extern unsigned char D_800BCFEE;
extern unsigned char D_800BCFEF;
extern unsigned short D_800BCFF0;
extern unsigned short D_800BCFF2;
extern unsigned short D_800BCFF4;
extern short D_800BCFF6;
extern unsigned short D_800BCFF8;

int func_80066BD8(short arg0, unsigned short arg1, unsigned short arg2, unsigned short arg3, unsigned short arg4) {
    unsigned short a = D_800BCFE8;
    unsigned short b = D_800BCFEA;
    unsigned short c = D_800BCFEC;
    int v = arg4 & 0xFFFF;
    D_800BCFE8 = arg1;
    D_800BCFEA = arg2;
    D_800BCFEC = arg3;
    D_800BCFEE = 2;
    D_800BCFF6 = arg0;
    D_800BCFF8 = 0;
    D_800BCFF0 = a;
    D_800BCFF2 = b;
    D_800BCFF4 = c;
    if (v < 4) {
        if (v >= 0) {
            D_800BCFEF = arg4;
            return 0;
        }
    }
    D_800BCFEF = 1;
    return 0;
}
