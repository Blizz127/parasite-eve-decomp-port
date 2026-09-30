/* room_m0087i (PE.IMG room m0087i chunk 2, VRAM 0x8018EFE8)
 * func_8018F394 — blob offset 0x3ac, 0x144 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F1BC; C re-targeted by symbol address
 * (docs/evidence/room_m0087i-ports-2026-09-23/REPORT.md). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { int w[8]; } Blk;
extern void func_800C2B40();
extern int func_8006DC18();
extern int *func_800C2B28();
extern short D_80193640;
extern unsigned char D_8019362C;
extern unsigned char D_8019362D;
extern unsigned char D_8019363C;
extern unsigned char D_8019363D;
extern short D_80193642;
extern unsigned char D_80193638;
extern unsigned char D_80193639;
extern unsigned char D_8019363A;
extern unsigned char D_8019363E;
extern short D_80193630;
extern short D_80193632;
extern unsigned char D_80193628;
extern unsigned char D_80193629;
extern unsigned char D_8019362A;
extern unsigned char D_8019362E;
void func_8018F394(void *o, int a1, void *p)
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
    D_80193640 = -100;
    D_8019362C = 4;
    D_8019362D = 1;
    D_8019363C = 0;
    D_8019363D = 0;
    D_80193642 = 128;
    D_80193638 = 128;
    D_80193639 = 128;
    D_8019363A = 128;
    D_8019363E = 0;
    D_80193630 = 50;
    D_80193632 = 128;
    D_80193628 = 128;
    D_80193629 = 128;
    D_8019362A = 128;
    D_8019362E = 0;
}
