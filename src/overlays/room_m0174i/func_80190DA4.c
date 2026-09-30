/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_80190DA4 — blob offset 0x1DBC, 0x140 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Init the two fog records D_80197460[0..1] (buffers in D_80196FE8) and the effect sizes in p. */

typedef struct {
    unsigned char *buf;
    unsigned char r0, g0, b0, pad7;
    unsigned char r1, g1, b1, padB;
    short c, e, f10, f12, f14, pad16;
} Fog;
extern Fog D_80197460[2];
extern unsigned char D_80196FE8[];
extern void func_800C4E50();
void func_80190DA4(int a0, int a1, short *p)
{
    p[8] = 10;
    p[9] = 10;
    p[11] = 20;
    p[10] = 255;
    D_80197460[0].c = 16;
    D_80197460[0].f12 = 0;
    D_80197460[0].f14 = 128;
    D_80197460[0].r1 = 120;
    D_80197460[0].g1 = 100;
    D_80197460[0].b1 = 30;
    D_80197460[0].r0 = 0;
    D_80197460[0].g0 = 0;
    D_80197460[0].b0 = 0;
    D_80197460[0].e = 1700;
    D_80197460[0].f10 = 1300;
    D_80197460[0].buf = D_80196FE8;
    func_800C4E50(&D_80197460[0]);
    D_80197460[1].c = 16;
    D_80197460[1].f12 = 0;
    D_80197460[1].f14 = 128;
    D_80197460[1].r1 = 0;
    D_80197460[1].g1 = 0;
    D_80197460[1].b1 = 0;
    D_80197460[1].r0 = 120;
    D_80197460[1].g0 = 100;
    D_80197460[1].b0 = 30;
    D_80197460[1].f10 = 550;
    D_80197460[1].e = 1300;
    D_80197460[1].buf = D_80196FE8 + 256;
    func_800C4E50(&D_80197460[1]);
}
