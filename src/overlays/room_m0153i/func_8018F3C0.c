/* room_m0153i (PE.IMG room m0153i chunk 2, VRAM 0x8018EFE8)
 * func_8018F3C0 — blob offset 0x3d8, 0x244 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_8018F224; C re-targeted by symbol address
 * (docs/evidence/room_m0153i-ports-2026-09-23/REPORT.md). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

typedef struct { int w[8]; } S32;
extern unsigned char D_80195758, D_80195759, D_8019575A, D_8019575C, D_8019575D, D_8019575E;
extern unsigned char D_80195768, D_80195769, D_8019576A, D_8019576C, D_8019576D, D_8019576E;
extern unsigned char D_80195778, D_80195779, D_8019577A, D_8019577C, D_8019577D, D_8019577E;
extern unsigned char D_80195788, D_80195789, D_8019578A, D_8019578C, D_8019578D, D_8019578E;
extern short D_80195760, D_80195762, D_80195770, D_80195772, D_80195780, D_80195782, D_80195790, D_80195792;
extern short D_800942EC;
extern void func_800C2B40();
extern int *func_800C2B28();
extern int func_8006DC18();
extern void func_800C66C8();

void func_8018F3C0(void *a0, int a1, void *a2)
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
    D_8019575C = 0x42;
    D_8019575D = 3;
    D_80195760 = -0x32;
    D_80195762 = 0x80;
    D_80195758 = 0x80;
    D_8019578C = 0x20;
    D_8019578D = 1;
    D_80195790 = -0x33;
    D_8019577C = 0x40;
    D_8019577D = 2;
    D_80195780 = -0x29;
    D_8019576C = 0x47;
    D_80195792 = 0x80;
    D_80195782 = 0x80;
    D_80195772 = 0x80;
    D_80195770 = -0x29;
    D_80195759 = 0x80;
    D_8019575A = 0x80;
    D_8019575E = 0;
    D_80195788 = 0x80;
    D_80195789 = 0x80;
    D_8019578A = 0x80;
    D_8019578E = 0;
    D_80195778 = 0x80;
    D_80195779 = 0x80;
    D_8019577A = 0x80;
    D_8019577E = 0;
    D_8019576D = 5;
    D_80195768 = 0x80;
    D_80195769 = 0x80;
    D_8019576A = 0x80;
    D_8019576E = 0;
    D_800942EC = 0;
    func_800C66C8(a0, 0x587, (char *)a2 + 4);
}
