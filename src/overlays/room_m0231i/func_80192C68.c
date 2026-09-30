/* room_m0231i (PE.IMG room m0231i chunk 2, VRAM 0x8018EFE8)
 * func_80192C68 — blob offset 0x3c80, 0x24 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0014i func_80193148; C re-targeted by symbol address
 * (docs/evidence/room_m0231i-ports-2026-09-23/REPORT.md). */

extern int D_801948F4;
extern int D_801948F8;
extern int D_801948FC;
int *func_80192C68(int a0, int a1, int a2, int a3)
{
    int *p = &D_801948F4;

    *p = a1;
    D_801948F8 = a2;
    D_801948FC = a3;
    return p;
}
