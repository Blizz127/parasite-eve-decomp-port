/* room_m0269i (PE.IMG room m0269i chunk 2, VRAM 0x8018EFE8)
 * func_8019218C — blob offset 0x31a4, 0x10 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0123i func_801953D4; C re-targeted by symbol address
 * (docs/evidence/room_m0269i-ports-2026-09-23/REPORT.md). */

extern int D_80192330;
int *func_8019218C(int a0, int a1)
{
    int *p = &D_80192330;

    *p = a1;
    return p;
}
