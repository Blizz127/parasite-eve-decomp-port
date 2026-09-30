/* room_m0012i (PE.IMG room m0012i chunk 2, VRAM 0x8018EFE8)
 * func_8018FC54 — blob offset 0xc6c, 0x24 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0014i func_80193148; C re-targeted by symbol address
 * (docs/evidence/room_m0012i-ports-2026-09-23/REPORT.md). */

extern int D_8018FCEC;
extern int D_8018FCF0;
extern int D_8018FCF4;
int *func_8018FC54(int a0, int a1, int a2, int a3)
{
    int *p = &D_8018FCEC;

    *p = a1;
    D_8018FCF0 = a2;
    D_8018FCF4 = a3;
    return p;
}
