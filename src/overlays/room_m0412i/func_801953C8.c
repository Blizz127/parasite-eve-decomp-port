/* room_m0412i (PE.IMG room m0412i chunk 2, VRAM 0x8018EFE8)
 * func_801953C8 — blob offset 0x63e0, 0xa4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_80191F38; C re-targeted by symbol address
 * (docs/evidence/room_m0412i-ports-2026-09-23/REPORT.md). */

extern int D_801956D8;
extern int D_801956DC;
extern int D_801956E0;
extern int D_801956E4;
extern int D_801956E8;
extern int D_801956EC;
extern int D_801956F0;
extern int D_801956F4;
int func_801953C8(unsigned int a0, int a1, int a2, int a3)
{
    switch (a0) {
    case 0:
        D_801956D8 = a1;
        D_801956DC = a2;
        D_801956F4 = a3;
        if (a3 == 0) {
            D_801956F4 = 8;
        }
        break;
    case 1:
        D_801956E0 = a1;
        D_801956E4 = a2;
        D_801956F0 = a3;
        if (a3 == 0) {
            D_801956F0 = 10;
        }
        break;
    case 2:
        D_801956E8 = a1;
        D_801956EC = a2;
        break;
    }
    return 0;
}
