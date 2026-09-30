/* room_m0167i (PE.IMG room m0167i chunk 2, VRAM 0x8018EFE8)
 * func_8018F28C — blob offset 0x2A4, 0x8C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Two lookups when func_800C6CE0(o) >= 2; on -1 call func_8018F318; return 0. */

extern unsigned int func_800C6CE0();
extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F318();
extern char D_80193184[];
extern char D_8019319C[];
extern char D_801931A8[];
int func_8018F28C(void *o)
{
    int r;

    if (func_800C6CE0(o) >= 2) {
        r = func_800C251C(o, D_8019319C);
        r |= func_800C2758(o, D_80193184, D_801931A8);
    } else {
        r = -1;
    }
    if (r == -1) {
        func_8018F318(o);
    }
    return 0;
}
