/* room_m0065i (PE.IMG room m0065i chunk 2, VRAM 0x8018EFE8)
 * func_8018F1BC — blob offset 0x1d4, 0x144 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0391i func_8018F3A8; C re-targeted by symbol address
 * (docs/evidence/room_m0065i-ports-2026-09-23/REPORT.md). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { int w[8]; } Blk;
extern void func_800C2B40();
extern int func_8006DC18();
extern int *func_800C2B28();
extern short D_801900A0;
extern unsigned char D_8019008C;
extern unsigned char D_8019008D;
extern unsigned char D_8019009C;
extern unsigned char D_8019009D;
extern short D_801900A2;
extern unsigned char D_80190098;
extern unsigned char D_80190099;
extern unsigned char D_8019009A;
extern unsigned char D_8019009E;
extern short D_80190090;
extern short D_80190092;
extern unsigned char D_80190088;
extern unsigned char D_80190089;
extern unsigned char D_8019008A;
extern unsigned char D_8019008E;
void func_8018F1BC(void *o, int a1, void *p)
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
    D_801900A0 = -100;
    D_8019008C = 4;
    D_8019008D = 1;
    D_8019009C = 0;
    D_8019009D = 0;
    D_801900A2 = 128;
    D_80190098 = 128;
    D_80190099 = 128;
    D_8019009A = 128;
    D_8019009E = 0;
    D_80190090 = 50;
    D_80190092 = 128;
    D_80190088 = 128;
    D_80190089 = 128;
    D_8019008A = 128;
    D_8019008E = 0;
}
