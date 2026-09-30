/* room_m0419i (PE.IMG room m0419i chunk 2, VRAM 0x8018EFE8)
 * func_80194BEC — blob offset 0x5c04, 0x1c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0126i func_8018FCC0; C re-targeted by symbol address
 * (docs/evidence/room_m0419i-ports-2026-09-23/REPORT.md). */

extern int D_80194E00;
extern int D_80194E04;
int *func_80194BEC(int a0, int a1, int a2)
{
    int *p = &D_80194E00;

    *p = a1;
    D_80194E04 = a2;
    return p;
}
