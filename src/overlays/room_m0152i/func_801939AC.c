/* room_m0152i (PE.IMG room m0152i chunk 2, VRAM 0x8018EFE8)
 * func_801939AC — blob offset 0x49c4, 0x1c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0126i func_8018FCC0; C re-targeted by symbol address
 * (docs/evidence/room_m0152i-ports-2026-09-23/REPORT.md). */

extern int D_8019403C;
extern int D_80194040;
int *func_801939AC(int a0, int a1, int a2)
{
    int *p = &D_8019403C;

    *p = a1;
    D_80194040 = a2;
    return p;
}
