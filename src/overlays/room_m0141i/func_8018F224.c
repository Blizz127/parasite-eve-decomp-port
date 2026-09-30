/* room_m0141i — func_8018F224, blob offset 0x23C, 0x244 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * Trail object init + 4 colour-block global groups; global store order from a single-move climb then a pair-move search (47->0). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

typedef struct { int w[8]; } S32;
extern unsigned char D_801921A0, D_801921A1, D_801921A2, D_801921A4, D_801921A5, D_801921A6;
extern unsigned char D_801921B0, D_801921B1, D_801921B2, D_801921B4, D_801921B5, D_801921B6;
extern unsigned char D_801921C0, D_801921C1, D_801921C2, D_801921C4, D_801921C5, D_801921C6;
extern unsigned char D_801921D0, D_801921D1, D_801921D2, D_801921D4, D_801921D5, D_801921D6;
extern short D_801921A8, D_801921AA, D_801921B8, D_801921BA, D_801921C8, D_801921CA, D_801921D8, D_801921DA;
extern short D_800942EC;
extern void func_800C2B40();
extern int *func_800C2B28();
extern int func_8006DC18();
extern void func_800C66C8();

void func_8018F224(void *a0, int a1, void *a2)
{
    void *x;

    func_800C2B40(a2);
    H(a2, 0x2A) = 0;
    H(a2, 0x2C) = 0;
    H(a2, 0x28) = *func_800C2B28(5);
    H(a2, 0x2E) = *func_800C2B28(4);
    x = P(a0, 8);
    P(a2, 0) = x;
    *(S32 *)((char *)a2 + 4) = *(S32 *)P(x, 0x238);
    W(a2, 0x18) = *func_800C2B28(1);
    W(a2, 0x1C) = *func_800C2B28(2);
    W(a2, 0x20) = *func_800C2B28(3);
    W(a2, 0x24) = func_8006DC18(0xA6);
    D_801921A4 = 0x42;
    D_801921A5 = 3;
    D_801921A8 = -0x32;
    D_801921AA = 0x80;
    D_801921A0 = 0x80;
    D_801921D4 = 0x20;
    D_801921D5 = 1;
    D_801921D8 = -0x33;
    D_801921C4 = 0x40;
    D_801921C5 = 2;
    D_801921C8 = -0x29;
    D_801921B4 = 0x47;
    D_801921DA = 0x80;
    D_801921CA = 0x80;
    D_801921BA = 0x80;
    D_801921B8 = -0x29;
    D_801921A1 = 0x80;
    D_801921A2 = 0x80;
    D_801921A6 = 0;
    D_801921D0 = 0x80;
    D_801921D1 = 0x80;
    D_801921D2 = 0x80;
    D_801921D6 = 0;
    D_801921C0 = 0x80;
    D_801921C1 = 0x80;
    D_801921C2 = 0x80;
    D_801921C6 = 0;
    D_801921B5 = 5;
    D_801921B0 = 0x80;
    D_801921B1 = 0x80;
    D_801921B2 = 0x80;
    D_801921B6 = 0;
    D_800942EC = 0;
    func_800C66C8(a0, 0x587, (char *)a2 + 4);
}
