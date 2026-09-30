/* room_m0113i (PE.IMG room m0113i chunk 2, VRAM 0x8018EFE8)
 * func_8018FC68 — blob offset 0xc80, 0x24 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0012i func_8018FC54; C re-targeted by symbol address
 * (docs/evidence/room_m0113i-ports-2026-09-23/REPORT.md). */

extern int D_80190B8C;
extern int D_80190B90;
extern int D_80190B94;
int *func_8018FC68(int a0, int a1, int a2, int a3)
{
    int *p = &D_80190B8C;

    *p = a1;
    D_80190B90 = a2;
    D_80190B94 = a3;
    return p;
}
