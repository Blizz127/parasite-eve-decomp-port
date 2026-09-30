/* room_m0141i (PE.IMG room m0141i chunk 2, VRAM 0x8018EFE8)
 * func_8018F130 — blob offset 0x148, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0034i func_8018F12C; C re-targeted by symbol address
 * (docs/evidence/room_m0141i-ports-2026-09-23/REPORT.md). */

extern int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F1BC();
extern char D_80191FDC[];
extern char D_80192014[];
extern char D_80192030[];
int func_8018F130(void *o)
{
    int r;

    if (func_800C6CE0(o) == 3) {
        r = func_800C251C(o, D_80192014);
        r |= func_800C2758(o, D_80191FDC, D_80192030);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F1BC(o);
    }
    return 0;
}
