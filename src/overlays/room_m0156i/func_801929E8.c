/* room_m0156i (PE.IMG room m0156i chunk 2, VRAM 0x8018EFE8)
 * func_801929E8 — blob offset 0x3a00, 0x10 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0123i func_801953D4; C re-targeted by symbol address
 * (docs/evidence/room_m0156i-ports-2026-09-23/REPORT.md). */

extern int D_80192BF8;
int *func_801929E8(int a0, int a1)
{
    int *p = &D_80192BF8;

    *p = a1;
    return p;
}
