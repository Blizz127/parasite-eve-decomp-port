/* room_m0014i (PE.IMG room m0014i chunk 2, VRAM 0x8018EFE8)
 * func_80193148 — blob offset 0x4160, 0x24 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Store a1..a3 into D_80193288..D_80193290; the first store goes through the returned pointer. */

extern int D_80193288;
extern int D_8019328C;
extern int D_80193290;
int *func_80193148(int a0, int a1, int a2, int a3)
{
    int *p = &D_80193288;

    *p = a1;
    D_8019328C = a2;
    D_80193290 = a3;
    return p;
}
