/* room_m0318i (PE.IMG room m0318i chunk 2, VRAM 0x8018EFE8)
 * func_8019937C — blob offset 0xa394, 0x10 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0123i func_801953D4; C re-targeted by symbol address
 * (docs/evidence/room_m0318i-ports-2026-09-23/REPORT.md). */

extern int D_80199938;
int *func_8019937C(int a0, int a1)
{
    int *p = &D_80199938;

    *p = a1;
    return p;
}
