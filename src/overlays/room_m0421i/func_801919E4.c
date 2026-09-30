/* room_m0421i (PE.IMG room m0421i chunk 2, VRAM 0x8018EFE8)
 * func_801919E4 — blob offset 0x29fc, 0x1c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0126i func_8018FCC0; C re-targeted by symbol address
 * (docs/evidence/room_m0421i-ports-2026-09-23/REPORT.md). */

extern int D_80191BD0;
extern int D_80191BD4;
int *func_801919E4(int a0, int a1, int a2)
{
    int *p = &D_80191BD0;

    *p = a1;
    D_80191BD4 = a2;
    return p;
}
