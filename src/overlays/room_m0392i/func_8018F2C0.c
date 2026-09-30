/* room_m0392i (PE.IMG room m0392i chunk 2, VRAM 0x8018EFE8)
 * func_8018F2C0 — blob offset 0x2d8, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F09C; C re-targeted by symbol address
 * (docs/evidence/room_m0392i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F34C();
extern char D_80194484[];
extern char D_801944B4[];
extern char D_801944CC[];
int func_8018F2C0(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_801944B4);
        r |= func_800C2758(o, D_80194484, D_801944CC);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F34C(o);
    }
    return 0;
}
