/* room_m0421i (PE.IMG room m0421i chunk 2, VRAM 0x8018EFE8)
 * func_8018F210 — blob offset 0x228, 0x10C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Scene-object init: copy the ctx +0x238 block, seed the colour globals and the func_8006E498 handle. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { int w[8]; } Blk;
extern void func_800C2B40();
extern int func_8006DC18();
extern int func_8006E498();
extern int D_800B0E64;
extern short D_80191BE0;
extern short D_80191BE2;
extern unsigned char D_80191BDC;
extern unsigned char D_80191BDD;
extern unsigned char D_80191BD8;
extern unsigned char D_80191BD9;
extern unsigned char D_80191BDA;
extern unsigned char D_80191BDE;
extern int D_80191BE4;
void func_8018F210(void *o, int a1, void *p)
{
    void *c;

    func_800C2B40(p);
    c = P(o, 0x8);
    P(p, 0x0) = c;
    *(Blk *)((char *)p + 4) = *(Blk *)P(c, 0x238);
    H(p, 0x2A) = 0;
    H(p, 0x2C) = 0;
    H(p, 0x28) = 30;
    W(p, 0x24) = func_8006DC18(35);
    D_80191BE0 = -300;
    D_80191BE2 = 128;
    D_80191BDC = 0;
    D_80191BDD = 0;
    D_80191BD8 = 128;
    D_80191BD9 = 128;
    D_80191BDA = 128;
    D_80191BDE = 0;
    D_80191BE4 = func_8006E498(D_800B0E64, 0xCB8704);
}
