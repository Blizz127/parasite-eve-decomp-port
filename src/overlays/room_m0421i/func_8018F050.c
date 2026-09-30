/* room_m0421i (PE.IMG room m0421i chunk 2, VRAM 0x8018EFE8)
 * func_8018F050 — blob offset 0x68, 0x2C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Store the D_80191A70 pointer through func_800C22F8's result; return 0. */

extern int *func_800C22F8();
extern char D_80191A70[];
int func_8018F050(void)
{
    *func_800C22F8() = (int)D_80191A70;
    return 0;
}
