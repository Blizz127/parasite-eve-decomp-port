/* room_m0412i (PE.IMG room m0412i chunk 2, VRAM 0x8018EFE8)
 * func_8018F3C8 — blob offset 0x3e0, 0x244 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_8018F224; C re-targeted by symbol address
 * (docs/evidence/room_m0412i-ports-2026-09-23/REPORT.md). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

typedef struct { int w[8]; } S32;
extern unsigned char D_801956F8, D_801956F9, D_801956FA, D_801956FC, D_801956FD, D_801956FE;
extern unsigned char D_80195708, D_80195709, D_8019570A, D_8019570C, D_8019570D, D_8019570E;
extern unsigned char D_80195718, D_80195719, D_8019571A, D_8019571C, D_8019571D, D_8019571E;
extern unsigned char D_80195728, D_80195729, D_8019572A, D_8019572C, D_8019572D, D_8019572E;
extern short D_80195700, D_80195702, D_80195710, D_80195712, D_80195720, D_80195722, D_80195730, D_80195732;
extern short D_800942EC;
extern void func_800C2B40();
extern int *func_800C2B28();
extern int func_8006DC18();
extern void func_800C66C8();

void func_8018F3C8(void *a0, int a1, void *a2)
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
    D_801956FC = 0x42;
    D_801956FD = 3;
    D_80195700 = -0x32;
    D_80195702 = 0x80;
    D_801956F8 = 0x80;
    D_8019572C = 0x20;
    D_8019572D = 1;
    D_80195730 = -0x33;
    D_8019571C = 0x40;
    D_8019571D = 2;
    D_80195720 = -0x29;
    D_8019570C = 0x47;
    D_80195732 = 0x80;
    D_80195722 = 0x80;
    D_80195712 = 0x80;
    D_80195710 = -0x29;
    D_801956F9 = 0x80;
    D_801956FA = 0x80;
    D_801956FE = 0;
    D_80195728 = 0x80;
    D_80195729 = 0x80;
    D_8019572A = 0x80;
    D_8019572E = 0;
    D_80195718 = 0x80;
    D_80195719 = 0x80;
    D_8019571A = 0x80;
    D_8019571E = 0;
    D_8019570D = 5;
    D_80195708 = 0x80;
    D_80195709 = 0x80;
    D_8019570A = 0x80;
    D_8019570E = 0;
    D_800942EC = 0;
    func_800C66C8(a0, 0x587, (char *)a2 + 4);
}
