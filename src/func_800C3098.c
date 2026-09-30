/*
 * func_800C3098 — display-mode selector + mode-record refresh (retail 0x800C3098).
 *
 * VRAM 0x800C3098 / file 0xB3898 / size 0x9C (39 words), inside B3390.s.
 *
 * switch (s16 arg0): 0x10 -> D_800F33AC = 0, 0x100 -> D_800F33AC = 1,
 * anything else calls func_80071A74(&D_800C2110). Then it recomputes the
 * display record D_800E27AC from func_80077A64(D_800F33AC, D_800E224C,
 * D_800F3424, D_800F3426).
 *
 * era -O2 -G0. The function is **void**: retail stores the call result
 * with `sh $v0,%lo(D_800E27AC)` and returns `$v0` unmodified. An
 * `unsigned short`/`int` return makes cc1 add andi/sll/sra extension
 * around the store (10-11 word diffs).
 */
extern unsigned char D_800F33AC;
extern unsigned char D_800E224C;
extern unsigned short D_800F3424;
extern unsigned short D_800F3426;
extern unsigned short D_800E27AC;
extern unsigned char D_800C2110[];
extern void func_80071A74(unsigned char *);
extern unsigned short func_80077A64(unsigned char, unsigned char, unsigned short, unsigned short);

void func_800C3098(short arg0) {
    switch (arg0) {
    case 0x10:
        D_800F33AC = 0;
        break;
    case 0x100:
        D_800F33AC = 1;
        break;
    default:
        func_80071A74(D_800C2110);
        break;
    }
    D_800E27AC = func_80077A64(D_800F33AC, D_800E224C, D_800F3424, D_800F3426);
}
