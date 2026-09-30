/* room_m0162i (PE.IMG room m0162i chunk 2, VRAM 0x8018EFE8)
 * func_80193118 — blob offset 0x4130, 0x24 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0014i func_80193148; C re-targeted by symbol address
 * (docs/evidence/room_m0162i-ports-2026-09-23/REPORT.md). */

extern int D_8019328C;
extern int D_80193290;
extern int D_80193294;
int *func_80193118(int a0, int a1, int a2, int a3)
{
    int *p = &D_8019328C;

    *p = a1;
    D_80193290 = a2;
    D_80193294 = a3;
    return p;
}
