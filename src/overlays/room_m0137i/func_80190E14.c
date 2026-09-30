/* room_m0137i (PE.IMG room m0137i chunk 2, VRAM 0x8018EFE8)
 * func_80190E14 — blob offset 0x1e2c, 0x1c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0126i func_8018FCC0; C re-targeted by symbol address
 * (docs/evidence/room_m0137i-ports-2026-09-23/REPORT.md). */

extern int D_80190F48;
extern int D_80190F4C;
int *func_80190E14(int a0, int a1, int a2)
{
    int *p = &D_80190F48;

    *p = a1;
    D_80190F4C = a2;
    return p;
}
