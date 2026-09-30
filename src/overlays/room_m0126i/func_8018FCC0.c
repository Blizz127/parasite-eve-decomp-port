/* room_m0126i (PE.IMG room m0126i chunk 2, VRAM 0x8018EFE8)
 * func_8018FCC0 — blob offset 0xCD8, 0x1C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Store a two-word pair; the first store goes through the returned pointer. */

extern int D_8018FD38;
extern int D_8018FD3C;
int *func_8018FCC0(int a0, int a1, int a2)
{
    int *p = &D_8018FD38;

    *p = a1;
    D_8018FD3C = a2;
    return p;
}
