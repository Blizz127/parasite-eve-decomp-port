/* room_m0141i (PE.IMG room m0141i chunk 2, VRAM 0x8018EFE8)
 * func_80191F38 — blob offset 0x2F50, 0xA4 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Three-slot parameter setter (switch on unsigned a0; defaults 8 / 10 for a zero third word). */

extern int D_80192180;
extern int D_80192184;
extern int D_80192188;
extern int D_8019218C;
extern int D_80192190;
extern int D_80192194;
extern int D_80192198;
extern int D_8019219C;
int func_80191F38(unsigned int a0, int a1, int a2, int a3)
{
    switch (a0) {
    case 0:
        D_80192180 = a1;
        D_80192184 = a2;
        D_8019219C = a3;
        if (a3 == 0) {
            D_8019219C = 8;
        }
        break;
    case 1:
        D_80192188 = a1;
        D_8019218C = a2;
        D_80192198 = a3;
        if (a3 == 0) {
            D_80192198 = 10;
        }
        break;
    case 2:
        D_80192190 = a1;
        D_80192194 = a2;
        break;
    }
    return 0;
}
