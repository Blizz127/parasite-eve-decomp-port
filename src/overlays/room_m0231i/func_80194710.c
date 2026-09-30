/* room_m0231i (PE.IMG room m0231i chunk 2, VRAM 0x8018EFE8)
 * func_80194710 — blob offset 0x5728, 0x10 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0123i func_801953D4; C re-targeted by symbol address
 * (docs/evidence/room_m0231i-ports-2026-09-23/REPORT.md). */

extern int D_80194900;
int *func_80194710(int a0, int a1)
{
    int *p = &D_80194900;

    *p = a1;
    return p;
}
