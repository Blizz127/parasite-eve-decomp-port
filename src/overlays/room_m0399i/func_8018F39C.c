/* room_m0399i (PE.IMG room m0399i chunk 2, VRAM 0x8018EFE8)
 * func_8018F39C — blob offset 0x3b4, 0x144 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F1BC; C re-targeted by symbol address
 * (docs/evidence/room_m0399i-ports-2026-09-23/REPORT.md). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { int w[8]; } Blk;
extern void func_800C2B40();
extern int func_8006DC18();
extern int *func_800C2B28();
extern short D_801943A8;
extern unsigned char D_80194394;
extern unsigned char D_80194395;
extern unsigned char D_801943A4;
extern unsigned char D_801943A5;
extern short D_801943AA;
extern unsigned char D_801943A0;
extern unsigned char D_801943A1;
extern unsigned char D_801943A2;
extern unsigned char D_801943A6;
extern short D_80194398;
extern short D_8019439A;
extern unsigned char D_80194390;
extern unsigned char D_80194391;
extern unsigned char D_80194392;
extern unsigned char D_80194396;
void func_8018F39C(void *o, int a1, void *p)
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
    D_801943A8 = -100;
    D_80194394 = 4;
    D_80194395 = 1;
    D_801943A4 = 0;
    D_801943A5 = 0;
    D_801943AA = 128;
    D_801943A0 = 128;
    D_801943A1 = 128;
    D_801943A2 = 128;
    D_801943A6 = 0;
    D_80194398 = 50;
    D_8019439A = 128;
    D_80194390 = 128;
    D_80194391 = 128;
    D_80194392 = 128;
    D_80194396 = 0;
}
