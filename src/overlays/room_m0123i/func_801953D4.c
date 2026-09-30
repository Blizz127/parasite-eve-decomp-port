/* room_m0123i (PE.IMG room m0123i chunk 2, VRAM 0x8018EFE8)
 * func_801953D4 — blob offset 0x63ec, 0x10 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0203i func_80193FC0; C re-targeted by symbol address
 * (docs/evidence/room_m0123i-ports-2026-09-23/REPORT.md). */

extern int D_801956BC;
int *func_801953D4(int a0, int a1)
{
    int *p = &D_801956BC;

    *p = a1;
    return p;
}
