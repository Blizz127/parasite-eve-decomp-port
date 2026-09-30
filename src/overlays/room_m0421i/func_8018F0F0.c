/* room_m0421i (PE.IMG room m0421i chunk 2, VRAM 0x8018EFE8)
 * func_8018F0F0 — blob offset 0x108, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F09C; C re-targeted by symbol address
 * (docs/evidence/room_m0421i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F17C();
extern char D_80191A00[];
extern char D_80191A30[];
extern char D_80191A48[];
int func_8018F0F0(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_80191A30);
        r |= func_800C2758(o, D_80191A00, D_80191A48);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F17C(o);
    }
    return 0;
}
