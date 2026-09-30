/* room_m0404i (PE.IMG room m0404i chunk 2, VRAM 0x8018EFE8)
 * func_80193E24 — blob offset 0x4e3c, 0x10 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0123i func_801953D4; C re-targeted by symbol address
 * (docs/evidence/room_m0404i-ports-2026-09-23/REPORT.md). */

extern int D_80193FC0;
int *func_80193E24(int a0, int a1)
{
    int *p = &D_80193FC0;

    *p = a1;
    return p;
}
