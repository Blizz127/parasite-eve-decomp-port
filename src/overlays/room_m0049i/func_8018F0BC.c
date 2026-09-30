/* room_m0049i (PE.IMG room m0049i chunk 2, VRAM 0x8018EFE8)
 * func_8018F0BC — blob offset 0xD4, 0x70 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Two lookups OR-ed in one expression; on -1 call func_8018F12C; return 0. */

extern int func_800C251C();
extern int func_800C2758();
extern void func_8018F12C();
extern char D_80192B5C[];
extern char D_80192B9C[];
extern char D_80192BBC[];
int func_8018F0BC(void *o)
{
    int r;

    r = func_800C251C(o, D_80192B9C) | func_800C2758(o, D_80192B5C, D_80192BBC);
    if (r == -1) {
        func_8018F12C(o);
    }
    return 0;
}
