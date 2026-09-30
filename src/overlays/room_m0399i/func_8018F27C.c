/* room_m0399i (PE.IMG room m0399i chunk 2, VRAM 0x8018EFE8)
 * func_8018F27C — blob offset 0x294, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F09C; C re-targeted by symbol address
 * (docs/evidence/room_m0399i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F308();
extern char D_801941C0[];
extern char D_801941D8[];
extern char D_801941E4[];
int func_8018F27C(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_801941D8);
        r |= func_800C2758(o, D_801941C0, D_801941E4);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F308(o);
    }
    return 0;
}
