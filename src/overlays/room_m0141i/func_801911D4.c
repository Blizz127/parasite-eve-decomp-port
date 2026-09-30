/* room_m0141i (PE.IMG room m0141i chunk 2, VRAM 0x8018EFE8)
 * func_801911D4 — blob offset 0x21ec, 0x1c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0126i func_8018FCC0; C re-targeted by symbol address
 * (docs/evidence/room_m0141i-ports-2026-09-23/REPORT.md). */

extern int D_80192178;
extern int D_8019217C;
int *func_801911D4(int a0, int a1, int a2)
{
    int *p = &D_80192178;

    *p = a1;
    D_8019217C = a2;
    return p;
}
