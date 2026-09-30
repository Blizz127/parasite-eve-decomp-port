/* room_m0414i (PE.IMG room m0414i chunk 2, VRAM 0x8018EFE8)
 * func_80191168 — blob offset 0x2180, 0x24 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0014i func_80193148; C re-targeted by symbol address
 * (docs/evidence/room_m0414i-ports-2026-09-23/REPORT.md). */

extern int D_80192130;
extern int D_80192134;
extern int D_80192138;
int *func_80191168(int a0, int a1, int a2, int a3)
{
    int *p = &D_80192130;

    *p = a1;
    D_80192134 = a2;
    D_80192138 = a3;
    return p;
}
