/* room_m0146i (PE.IMG room m0146i chunk 2, VRAM 0x8018EFE8)
 * func_8018F124 — blob offset 0x13c, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0034i func_8018F12C; C re-targeted by symbol address
 * (docs/evidence/room_m0146i-ports-2026-09-23/REPORT.md). */

extern int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F1B0();
extern char D_80192448[];
extern char D_80192480[];
extern char D_8019249C[];
int func_8018F124(void *o)
{
    int r;

    if (func_800C6CE0(o) == 3) {
        r = func_800C251C(o, D_80192480);
        r |= func_800C2758(o, D_80192448, D_8019249C);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F1B0(o);
    }
    return 0;
}
