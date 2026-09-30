/* ovl_0700 (PE.IMG subsystem overlay, VRAM 0x8018EFF0)
 * func_80191854 — blob offset 0x2864, 0x440 bytes. Profile era_o2_g0 (default).
 * Map-exit setup: picks the backdrop index D_8019CC52 from the current map id and the
 * exit-slot pairs D_8019BFF4..D_8019C004 from the destination id, both read from the game
 * state struct at D_800A77F4 (cur +0, flags +8, dest +0x124 = D_800A7918). Plain if-chains;
 * cc1's cse path limits reproduce retail's reloads. Evidence: docs/evidence/ovl8-lane-2026-09-28/REPORT.md */
typedef struct {
    int cur;
    int pad4;
    int flags;
    unsigned char pad0C[0x118];
    int dest;
} GS;

extern GS D_800A77F4;
extern int D_8019C008;
extern unsigned int D_800B0CD8;
extern short D_8019CC52;
extern int D_8019BFF4;
extern int D_8019BFF8;
extern int D_8019BFFC;
extern int D_8019C000;
extern int D_8019C004;
extern unsigned char D_8019C1F0;
extern void func_8019BD78();
extern int func_8005BCB0();
extern void func_800371A4();
extern void func_80191C94();

void func_80191854(void)
{
    func_8019BD78();
    if (func_8005BCB0() == 0) {
        D_8019C008 = 0;
    } else {
        D_8019C008 = 0xFF;
    }
    if (D_800B0CD8 & 0x40000000) {
        D_8019C008 = 0xFF;
        func_800371A4(1);
    } else {
        D_8019C008 = 0;
        func_800371A4(0);
    }
    D_8019CC52 = -1;
    if (D_800A77F4.cur == 0x179) D_8019CC52 = 6;
    if (D_800A77F4.cur == 0x18) D_8019CC52 = 6;
    if (D_800A77F4.cur == 0x2E) D_8019CC52 = 7;
    if (D_800A77F4.cur == 0x75) D_8019CC52 = 7;
    if (D_800A77F4.cur == 0x67) D_8019CC52 = 7;
    if (D_800A77F4.cur == 0xBF) D_8019CC52 = 9;
    if (D_800A77F4.cur == 0xC0) D_8019CC52 = 9;
    if (D_800A77F4.cur == 0x5D) D_8019CC52 = 2;
    if (D_800A77F4.cur == 0x60) D_8019CC52 = 2;
    if (D_800A77F4.cur == 0x3A) D_8019CC52 = 8;
    if (D_800A77F4.cur == 0x176) D_8019CC52 = 8;
    if (D_800A77F4.cur == 0x7D) D_8019CC52 = 4;
    if (D_800A77F4.cur == 0x104) D_8019CC52 = 3;
    if (D_800A77F4.cur == 0xA1) D_8019CC52 = 1;
    if (D_800A77F4.cur == 0xB7) D_8019CC52 = 0;
    if (D_800A77F4.cur == 0x122) D_8019CC52 = 5;
    if (D_8019CC52 == -1) D_8019CC52 = 7;
    if (D_800A77F4.dest == 0x80) D_8019BFF4 = 0;
    if (D_800A77F4.dest == 0x208) D_8019BFF4 = 7;
    if (D_800A77F4.dest == 0xB8) { D_8019C000 = 1; D_8019C004 = 9; }
    if (D_800A77F4.dest == 0xC0) { D_8019BFF8 = 1; D_8019BFFC = 9; }
    if (D_800A77F4.dest == 0xD0) { D_8019C000 = 1; D_8019C004 = 7; }
    if (D_800A77F4.dest == 0xD8) { D_8019BFF8 = 1; D_8019BFFC = 7; }
    if (D_800A77F4.dest == 0xE0) { D_8019C000 = 1; D_8019C004 = 8; }
    if (D_800A77F4.dest == 0xE4) { D_8019BFF8 = 1; D_8019BFFC = 8; }
    if (D_800A77F4.dest == 0x148) { D_8019C000 = 1; D_8019C004 = 9; }
    if (D_800A77F4.dest == 0x160) { D_8019BFF8 = 1; D_8019BFFC = 9; }
    if (D_800A77F4.dest == 0x178) { D_8019C000 = 1; D_8019C004 = 7; }
    if (D_800A77F4.dest == 0x180) { D_8019BFF8 = 1; D_8019BFFC = 7; }
    if (D_800A77F4.dest == 0x1C0) { D_8019C000 = 1; D_8019C004 = 4; }
    if (D_800A77F4.dest == 0x1C8) { D_8019BFF8 = 1; D_8019BFFC = 4; }
    if (D_800A77F4.flags & 0x2000) {
        D_8019C1F0 = 1;
    } else {
        D_8019C1F0 = 0;
    }
    func_80191C94();
}
