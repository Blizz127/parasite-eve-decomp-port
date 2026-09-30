/* room_m0318i (PE.IMG room m0318i chunk 2, VRAM 0x8018EFE8)
 * func_80199294 — blob offset 0xa2ac, 0x10 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0123i func_801953D4; C re-targeted by symbol address
 * (docs/evidence/room_m0318i-ports-2026-09-23/REPORT.md). */

extern int D_8019992C;
int *func_80199294(int a0, int a1)
{
    int *p = &D_8019992C;

    *p = a1;
    return p;
}
