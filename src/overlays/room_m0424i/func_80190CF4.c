/* room_m0424i (PE.IMG room m0424i chunk 2, VRAM 0x8018EFE8)
 * func_80190CF4 — blob offset 0x1d0c, 0x24 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0014i func_80193148; C re-targeted by symbol address
 * (docs/evidence/room_m0424i-ports-2026-09-23/REPORT.md). */

extern int D_80190E80;
extern int D_80190E84;
extern int D_80190E88;
int *func_80190CF4(int a0, int a1, int a2, int a3)
{
    int *p = &D_80190E80;

    *p = a1;
    D_80190E84 = a2;
    D_80190E88 = a3;
    return p;
}
