/* room_m0186i (PE.IMG room m0186i chunk 2, VRAM 0x8018EFE8)
 * func_8018FF30 — blob offset 0xF48, 0x8C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Two lookups when func_800C6CE0(o) == 3; on -1 call func_8018FFBC; return 0. */

extern int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018FFBC();
extern char D_801941CC[];
extern char D_801941F4[];
extern char D_80194208[];
int func_8018FF30(void *o)
{
    int r;

    if (func_800C6CE0(o) == 3) {
        r = func_800C251C(o, D_801941F4);
        r |= func_800C2758(o, D_801941CC, D_80194208);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018FFBC(o);
    }
    return 0;
}
