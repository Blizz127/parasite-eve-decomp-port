/* room_m0393i (PE.IMG room m0393i chunk 2, VRAM 0x8018EFE8)
 * func_8018F3CC — blob offset 0x3e4, 0x144 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F1BC; C re-targeted by symbol address
 * (docs/evidence/room_m0393i-ports-2026-09-23/REPORT.md). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { int w[8]; } Blk;
extern void func_800C2B40();
extern int func_8006DC18();
extern int *func_800C2B28();
extern short D_80194870;
extern unsigned char D_8019485C;
extern unsigned char D_8019485D;
extern unsigned char D_8019486C;
extern unsigned char D_8019486D;
extern short D_80194872;
extern unsigned char D_80194868;
extern unsigned char D_80194869;
extern unsigned char D_8019486A;
extern unsigned char D_8019486E;
extern short D_80194860;
extern short D_80194862;
extern unsigned char D_80194858;
extern unsigned char D_80194859;
extern unsigned char D_8019485A;
extern unsigned char D_8019485E;
void func_8018F3CC(void *o, int a1, void *p)
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
    D_80194870 = -100;
    D_8019485C = 4;
    D_8019485D = 1;
    D_8019486C = 0;
    D_8019486D = 0;
    D_80194872 = 128;
    D_80194868 = 128;
    D_80194869 = 128;
    D_8019486A = 128;
    D_8019486E = 0;
    D_80194860 = 50;
    D_80194862 = 128;
    D_80194858 = 128;
    D_80194859 = 128;
    D_8019485A = 128;
    D_8019485E = 0;
}
