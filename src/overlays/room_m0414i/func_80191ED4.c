/* room_m0414i (PE.IMG room m0414i chunk 2, VRAM 0x8018EFE8)
 * func_80191ED4 — blob offset 0x2eec, 0xa4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_80191F38; C re-targeted by symbol address
 * (docs/evidence/room_m0414i-ports-2026-09-23/REPORT.md). */

extern int D_80192140;
extern int D_80192144;
extern int D_80192148;
extern int D_8019214C;
extern int D_80192150;
extern int D_80192154;
extern int D_80192158;
extern int D_8019215C;
int func_80191ED4(unsigned int a0, int a1, int a2, int a3)
{
    switch (a0) {
    case 0:
        D_80192140 = a1;
        D_80192144 = a2;
        D_8019215C = a3;
        if (a3 == 0) {
            D_8019215C = 8;
        }
        break;
    case 1:
        D_80192148 = a1;
        D_8019214C = a2;
        D_80192158 = a3;
        if (a3 == 0) {
            D_80192158 = 10;
        }
        break;
    case 2:
        D_80192150 = a1;
        D_80192154 = a2;
        break;
    }
    return 0;
}
