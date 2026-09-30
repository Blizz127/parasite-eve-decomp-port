/* room_m0188i (PE.IMG room m0188i chunk 2, VRAM 0x8018EFE8)
 * func_80195D78 — blob offset 0x6d90, 0x10 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0123i func_801953D4; C re-targeted by symbol address
 * (docs/evidence/room_m0188i-ports-2026-09-23/REPORT.md). */

extern int D_80195FD0;
int *func_80195D78(int a0, int a1)
{
    int *p = &D_80195FD0;

    *p = a1;
    return p;
}
