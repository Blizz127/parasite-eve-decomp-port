/* room_m0153i (PE.IMG room m0153i chunk 2, VRAM 0x8018EFE8)
 * func_80195418 — blob offset 0x6430, 0xa4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_80191F38; C re-targeted by symbol address
 * (docs/evidence/room_m0153i-ports-2026-09-23/REPORT.md). */

extern int D_80195738;
extern int D_8019573C;
extern int D_80195740;
extern int D_80195744;
extern int D_80195748;
extern int D_8019574C;
extern int D_80195750;
extern int D_80195754;
int func_80195418(unsigned int a0, int a1, int a2, int a3)
{
    switch (a0) {
    case 0:
        D_80195738 = a1;
        D_8019573C = a2;
        D_80195754 = a3;
        if (a3 == 0) {
            D_80195754 = 8;
        }
        break;
    case 1:
        D_80195740 = a1;
        D_80195744 = a2;
        D_80195750 = a3;
        if (a3 == 0) {
            D_80195750 = 10;
        }
        break;
    case 2:
        D_80195748 = a1;
        D_8019574C = a2;
        break;
    }
    return 0;
}
