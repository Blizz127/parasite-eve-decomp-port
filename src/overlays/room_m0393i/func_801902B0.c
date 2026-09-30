/* room_m0393i (PE.IMG room m0393i chunk 2, VRAM 0x8018EFE8)
 * func_801902B0 — blob offset 0x12c8, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F09C; C re-targeted by symbol address
 * (docs/evidence/room_m0393i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8019033C();
extern char D_801946D0[];
extern char D_80194700[];
extern char D_80194718[];
int func_801902B0(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_80194700);
        r |= func_800C2758(o, D_801946D0, D_80194718);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8019033C(o);
    }
    return 0;
}
