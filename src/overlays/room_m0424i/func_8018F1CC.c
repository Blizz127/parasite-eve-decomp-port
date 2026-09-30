/* room_m0424i (PE.IMG room m0424i chunk 2, VRAM 0x8018EFE8)
 * func_8018F1CC — blob offset 0x1e4, 0x10c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0107i func_8018F394; C re-targeted by symbol address
 * (docs/evidence/room_m0424i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { int w[8]; } Blk;
extern void func_800C2B40();
extern int func_8006DC18();
extern int func_8006E498();
extern int D_800B0E64;
extern short D_80190E98;
extern short D_80190E9A;
extern unsigned char D_80190E94;
extern unsigned char D_80190E95;
extern unsigned char D_80190E90;
extern unsigned char D_80190E91;
extern unsigned char D_80190E92;
extern unsigned char D_80190E96;
extern int D_80190E9C;
void func_8018F1CC(void *o, int a1, void *p)
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
    D_80190E98 = -300;
    D_80190E9A = 128;
    D_80190E94 = 0;
    D_80190E95 = 0;
    D_80190E90 = 128;
    D_80190E91 = 128;
    D_80190E92 = 128;
    D_80190E96 = 0;
    D_80190E9C = func_8006E498(D_800B0E64, 0xCB8704);
}
