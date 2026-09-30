/* room_m0269i (PE.IMG room m0269i chunk 2, VRAM 0x8018EFE8)
 * func_801921AC — blob offset 0x31c4, 0x1c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0126i func_8018FCC0; C re-targeted by symbol address
 * (docs/evidence/room_m0269i-ports-2026-09-23/REPORT.md). */

extern int D_8019233C;
extern int D_80192340;
int *func_801921AC(int a0, int a1, int a2)
{
    int *p = &D_8019233C;

    *p = a1;
    D_80192340 = a2;
    return p;
}
