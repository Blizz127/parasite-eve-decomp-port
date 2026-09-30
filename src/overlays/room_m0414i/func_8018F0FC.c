/* room_m0414i (PE.IMG room m0414i chunk 2, VRAM 0x8018EFE8)
 * func_8018F0FC — blob offset 0x114, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0034i func_8018F12C; C re-targeted by symbol address
 * (docs/evidence/room_m0414i-ports-2026-09-23/REPORT.md). */

extern int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F188();
extern char D_80191F78[];
extern char D_80191FB0[];
extern char D_80191FCC[];
int func_8018F0FC(void *o)
{
    int r;

    if (func_800C6CE0(o) == 3) {
        r = func_800C251C(o, D_80191FB0);
        r |= func_800C2758(o, D_80191F78, D_80191FCC);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F188(o);
    }
    return 0;
}
