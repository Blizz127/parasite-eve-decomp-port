/* room_m0146i (PE.IMG room m0146i chunk 2, VRAM 0x8018EFE8)
 * func_801923A4 — blob offset 0x33bc, 0xa4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_80191F38; C re-targeted by symbol address
 * (docs/evidence/room_m0146i-ports-2026-09-23/REPORT.md). */

extern int D_80192648;
extern int D_8019264C;
extern int D_80192650;
extern int D_80192654;
extern int D_80192658;
extern int D_8019265C;
extern int D_80192660;
extern int D_80192664;
int func_801923A4(unsigned int a0, int a1, int a2, int a3)
{
    switch (a0) {
    case 0:
        D_80192648 = a1;
        D_8019264C = a2;
        D_80192664 = a3;
        if (a3 == 0) {
            D_80192664 = 8;
        }
        break;
    case 1:
        D_80192650 = a1;
        D_80192654 = a2;
        D_80192660 = a3;
        if (a3 == 0) {
            D_80192660 = 10;
        }
        break;
    case 2:
        D_80192658 = a1;
        D_8019265C = a2;
        break;
    }
    return 0;
}
