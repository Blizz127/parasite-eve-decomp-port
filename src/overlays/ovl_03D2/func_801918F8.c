/* ovl_03D2 (PE.IMG subsystem overlay, VRAM 0x8018EFF0)
 * func_801918F8 — blob offset 0x2908, 0x26C bytes. Profile era_o2_g0 (default).
 * Split-screen window setup for player slot a0: picks the {0xF0,0} / {0,0xF0} offsets, then
 * re-inits the 20-byte D_800BCE80 and 92-byte D_800BCDC8 records through func_800749D8 /
 * func_80074924 (full width 0x1E0 with the f4 = f4*2/3 rescale when a1, else 0x140) and resets
 * the record flag bytes. First draft (docs/evidence/ovl6-lane-2026-09-27/REPORT.md). */
typedef struct {
    unsigned char pad0[4];
    short f4;              /* +0x04 */
    unsigned char pad6[0xB];
    unsigned char f11;     /* +0x11 */
    unsigned char pad12[2];
} A20;
typedef struct {
    unsigned char pad0[4];
    short f4;              /* +0x04 */
    unsigned char pad6[0x10];
    unsigned char f16, f17, f18, f19, f1A, f1B; /* +0x16..0x1B */
    unsigned char pad1C[0x40];
} B92;
extern A20 D_800BCE80[];
extern B92 D_800BCDC8[];
extern unsigned char D_801D0DBE;
extern void func_800749D8();
extern void func_80074924();

void func_801918F8(signed char a0, signed char a1)
{
    short v[2];

    if (a0 == 0) {
        v[0] = 0xF0;
        v[1] = 0;
    } else {
        v[0] = 0;
        v[1] = 0xF0;
    }
    if (a1 != 0) {
        D_801D0DBE = 3;
        func_800749D8(&D_800BCE80[a0], 0, v[0], 0x1E0, 0xF0);
        D_800BCE80[a0].f4 = (D_800BCE80[a0].f4 * 2) / 3;
        D_800BCE80[a0].f11 = 1;
        func_80074924(&D_800BCDC8[a0], 0, v[1], 0x1E0, 0xF0);
        D_800BCDC8[a0].f4 = (D_800BCDC8[a0].f4 * 2) / 3;
        D_800BCDC8[a0].f18 = 1;
        D_800BCDC8[a0].f16 = 1;
        D_800BCDC8[a0].f17 = 0;
        D_800BCDC8[a0].f19 = 0;
        D_800BCDC8[a0].f1A = 0;
        D_800BCDC8[a0].f1B = 0;
    } else {
        D_801D0DBE = 2;
        func_800749D8(&D_800BCE80[a0], 0, v[0], 0x140, 0xF0);
        func_80074924(&D_800BCDC8[a0], 0, v[1], 0x140, 0xF0);
        D_800BCDC8[a0].f18 = 1;
        D_800BCDC8[a0].f16 = 1;
        D_800BCDC8[a0].f17 = 0;
        D_800BCDC8[a0].f19 = 0;
        D_800BCDC8[a0].f1A = 0;
        D_800BCDC8[a0].f1B = 0;
    }
}
