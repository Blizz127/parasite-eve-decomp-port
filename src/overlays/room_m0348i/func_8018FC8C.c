/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_8018FC8C — blob offset 0xca4, 0x140 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80190DA4; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

typedef struct {
    unsigned char *buf;
    unsigned char r0, g0, b0, pad7;
    unsigned char r1, g1, b1, padB;
    short c, e, f10, f12, f14, pad16;
} Fog;
extern Fog D_80192E50[2];
extern unsigned char D_80192A00[];
extern void func_800C4E50();
void func_8018FC8C(int a0, int a1, short *p)
{
    p[8] = 10;
    p[9] = 10;
    p[11] = 20;
    p[10] = 255;
    D_80192E50[0].c = 16;
    D_80192E50[0].f12 = 0;
    D_80192E50[0].f14 = 128;
    D_80192E50[0].r1 = 120;
    D_80192E50[0].g1 = 100;
    D_80192E50[0].b1 = 30;
    D_80192E50[0].r0 = 0;
    D_80192E50[0].g0 = 0;
    D_80192E50[0].b0 = 0;
    D_80192E50[0].e = 1700;
    D_80192E50[0].f10 = 1300;
    D_80192E50[0].buf = D_80192A00;
    func_800C4E50(&D_80192E50[0]);
    D_80192E50[1].c = 16;
    D_80192E50[1].f12 = 0;
    D_80192E50[1].f14 = 128;
    D_80192E50[1].r1 = 0;
    D_80192E50[1].g1 = 0;
    D_80192E50[1].b1 = 0;
    D_80192E50[1].r0 = 120;
    D_80192E50[1].g0 = 100;
    D_80192E50[1].b0 = 30;
    D_80192E50[1].f10 = 550;
    D_80192E50[1].e = 1300;
    D_80192E50[1].buf = D_80192A00 + 256;
    func_800C4E50(&D_80192E50[1]);
}
