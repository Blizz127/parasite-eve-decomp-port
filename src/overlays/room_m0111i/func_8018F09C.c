/* room_m0111i (PE.IMG room m0111i chunk 2, VRAM 0x8018EFE8)
 * func_8018F09C — blob offset 0xb4, 0x8c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F09C; C re-targeted by symbol address
 * (docs/evidence/room_m0111i-ports-2026-09-23/REPORT.md). */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F128();
extern char D_80190094[];
extern char D_801900C4[];
extern char D_801900DC[];
int func_8018F09C(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_801900C4);
        r |= func_800C2758(o, D_80190094, D_801900DC);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F128(o);
    }
    return 0;
}
