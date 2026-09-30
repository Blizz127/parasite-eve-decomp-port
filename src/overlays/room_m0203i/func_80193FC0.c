/* room_m0203i (PE.IMG room m0203i chunk 2, VRAM 0x8018EFE8)
 * func_80193FC0 — blob offset 0x4FD8, 0x10 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Store a1 into D_80194148 through the returned pointer. */

extern int D_80194148;
int *func_80193FC0(int a0, int a1)
{
    int *p = &D_80194148;

    *p = a1;
    return p;
}
