/* room_m0401i (PE.IMG room m0401i chunk 2, VRAM 0x8018EFE8)
 * func_8018F3D4 — blob offset 0x3ec, 0x144 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F1BC; C re-targeted by symbol address
 * (docs/evidence/room_m0401i-ports-2026-09-23/REPORT.md). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { int w[8]; } Blk;
extern void func_800C2B40();
extern int func_8006DC18();
extern int *func_800C2B28();
extern short D_801955D8;
extern unsigned char D_801955C4;
extern unsigned char D_801955C5;
extern unsigned char D_801955D4;
extern unsigned char D_801955D5;
extern short D_801955DA;
extern unsigned char D_801955D0;
extern unsigned char D_801955D1;
extern unsigned char D_801955D2;
extern unsigned char D_801955D6;
extern short D_801955C8;
extern short D_801955CA;
extern unsigned char D_801955C0;
extern unsigned char D_801955C1;
extern unsigned char D_801955C2;
extern unsigned char D_801955C6;
void func_8018F3D4(void *o, int a1, void *p)
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
    D_801955D8 = -100;
    D_801955C4 = 4;
    D_801955C5 = 1;
    D_801955D4 = 0;
    D_801955D5 = 0;
    D_801955DA = 128;
    D_801955D0 = 128;
    D_801955D1 = 128;
    D_801955D2 = 128;
    D_801955D6 = 0;
    D_801955C8 = 50;
    D_801955CA = 128;
    D_801955C0 = 128;
    D_801955C1 = 128;
    D_801955C2 = 128;
    D_801955C6 = 0;
}
