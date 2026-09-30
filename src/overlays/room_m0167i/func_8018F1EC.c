/* room_m0167i (PE.IMG room m0167i chunk 2, VRAM 0x8018EFE8)
 * func_8018F1EC — blob offset 0x204, 0x2C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Store the D_80193208 pointer through func_800C22F8's result; return 0. */

extern int *func_800C22F8();
extern char D_80193208[];
int func_8018F1EC(void)
{
    *func_800C22F8() = (int)D_80193208;
    return 0;
}
