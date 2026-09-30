/* room_m0421i (PE.IMG room m0421i chunk 2, VRAM 0x8018EFE8)
 * func_80190D38 — blob offset 0x1d50, 0x24 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0014i func_80193148; C re-targeted by symbol address
 * (docs/evidence/room_m0421i-ports-2026-09-23/REPORT.md). */

extern int D_80191BC0;
extern int D_80191BC4;
extern int D_80191BC8;
int *func_80190D38(int a0, int a1, int a2, int a3)
{
    int *p = &D_80191BC0;

    *p = a1;
    D_80191BC4 = a2;
    D_80191BC8 = a3;
    return p;
}
