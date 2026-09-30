/* room_m0263i (PE.IMG room m0263i chunk 2, VRAM 0x8018EFE8)
 * func_8019315C — blob offset 0x4174, 0x24 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0014i func_80193148; C re-targeted by symbol address
 * (docs/evidence/room_m0263i-ports-2026-09-23/REPORT.md). */

extern int D_80194128;
extern int D_8019412C;
extern int D_80194130;
int *func_8019315C(int a0, int a1, int a2, int a3)
{
    int *p = &D_80194128;

    *p = a1;
    D_8019412C = a2;
    D_80194130 = a3;
    return p;
}
