/* room_m0412i (PE.IMG room m0412i chunk 2, VRAM 0x8018EFE8)
 * func_8019465C — blob offset 0x5674, 0x24 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0014i func_80193148; C re-targeted by symbol address
 * (docs/evidence/room_m0412i-ports-2026-09-23/REPORT.md). */

extern int D_801956C8;
extern int D_801956CC;
extern int D_801956D0;
int *func_8019465C(int a0, int a1, int a2, int a3)
{
    int *p = &D_801956C8;

    *p = a1;
    D_801956CC = a2;
    D_801956D0 = a3;
    return p;
}
