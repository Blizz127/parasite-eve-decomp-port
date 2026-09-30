/* room_m0418i (PE.IMG room m0418i chunk 2, VRAM 0x8018EFE8)
 * func_80191460 — blob offset 0x2478, 0x70 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0049i func_8018F0BC; C re-targeted by symbol address
 * (docs/evidence/room_m0418i-ports-2026-09-23/REPORT.md). */

extern int func_800C251C();
extern int func_800C2758();
extern void func_801914D0();
extern char D_80198878[];
extern char D_801988D0[];
extern char D_801988FC[];
int func_80191460(void *o)
{
    int r;

    r = func_800C251C(o, D_801988D0) | func_800C2758(o, D_80198878, D_801988FC);
    if (r == -1) {
        func_801914D0(o);
    }
    return 0;
}
