/* room_m0205i (PE.IMG room m0205i chunk 2, VRAM 0x8018EFE8)
 * func_80192C14 — blob offset 0x3c2c, 0x24 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0014i func_80193148; C re-targeted by symbol address
 * (docs/evidence/room_m0205i-ports-2026-09-23/REPORT.md). */

extern int D_80192D3C;
extern int D_80192D40;
extern int D_80192D44;
int *func_80192C14(int a0, int a1, int a2, int a3)
{
    int *p = &D_80192D3C;

    *p = a1;
    D_80192D40 = a2;
    D_80192D44 = a3;
    return p;
}
