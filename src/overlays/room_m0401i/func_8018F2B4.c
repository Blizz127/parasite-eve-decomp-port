/* room_m0401i (PE.IMG room m0401i chunk 2, VRAM 0x8018EFE8)
 * func_8018F2B4 — blob offset 0x2cc, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F09C; C re-targeted by symbol address
 * (docs/evidence/room_m0401i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F340();
extern char D_80195314[];
extern char D_8019532C[];
extern char D_80195338[];
int func_8018F2B4(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_8019532C);
        r |= func_800C2758(o, D_80195314, D_80195338);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F340(o);
    }
    return 0;
}
