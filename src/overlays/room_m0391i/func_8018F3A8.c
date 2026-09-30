/* room_m0391i (PE.IMG room m0391i chunk 2, VRAM 0x8018EFE8)
 * func_8018F3A8 — blob offset 0x3C0, 0x144 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Scene-object init: copy the ctx +0x238 block, seed the light/colour globals. */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { int w[8]; } Blk;
extern void func_800C2B40();
extern int func_8006DC18();
extern int *func_800C2B28();
extern short D_80194560;
extern unsigned char D_8019454C;
extern unsigned char D_8019454D;
extern unsigned char D_8019455C;
extern unsigned char D_8019455D;
extern short D_80194562;
extern unsigned char D_80194558;
extern unsigned char D_80194559;
extern unsigned char D_8019455A;
extern unsigned char D_8019455E;
extern short D_80194550;
extern short D_80194552;
extern unsigned char D_80194548;
extern unsigned char D_80194549;
extern unsigned char D_8019454A;
extern unsigned char D_8019454E;
void func_8018F3A8(void *o, int a1, void *p)
{
    void *c;

    func_800C2B40(p);
    W(p, 0x2C) = func_8006DC18(10);
    c = P(o, 0x8);
    P(p, 0x0) = c;
    *(Blk *)((char *)p + 4) = *(Blk *)P(c, 0x238);
    H(p, 0x26) = 0;
    H(p, 0x28) = 0;
    H(p, 0x24) = *func_800C2B28(6);
    D_80194560 = -100;
    D_8019454C = 4;
    D_8019454D = 1;
    D_8019455C = 0;
    D_8019455D = 0;
    D_80194562 = 128;
    D_80194558 = 128;
    D_80194559 = 128;
    D_8019455A = 128;
    D_8019455E = 0;
    D_80194550 = 50;
    D_80194552 = 128;
    D_80194548 = 128;
    D_80194549 = 128;
    D_8019454A = 128;
    D_8019454E = 0;
}
