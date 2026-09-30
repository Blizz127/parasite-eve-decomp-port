/* room_m0391i (PE.IMG room m0391i chunk 2, VRAM 0x8018EFE8)
 * func_8018F288 — blob offset 0x2a0, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F09C; C re-targeted by symbol address
 * (docs/evidence/room_m0391i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F314();
extern char D_80194334[];
extern char D_8019434C[];
extern char D_80194358[];
int func_8018F288(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_8019434C);
        r |= func_800C2758(o, D_80194334, D_80194358);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F314(o);
    }
    return 0;
}
