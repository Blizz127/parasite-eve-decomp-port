/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_8018F240 — blob offset 0x258, 0x320 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80190358; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

typedef struct { int w[8]; } S32;
extern unsigned char D_80192E00, D_80192E01, D_80192E02, D_80192E04, D_80192E05, D_80192E06, D_80192E10, D_80192E11, D_80192E12, D_80192E14, D_80192E15, D_80192E16, D_80192E20, D_80192E21, D_80192E22, D_80192E24, D_80192E25, D_80192E26, D_80192E34, D_80192E35, D_80192E36, D_80192E40, D_80192E41, D_80192E42, D_80192E44, D_80192E45, D_80192E46, D_80192EB0, D_80192EB1, D_80192EB2, D_80192EB4, D_80192EB5, D_80192EB6, D_80192EC0, D_80192EC1, D_80192EC2, D_80192EC6;
extern short D_80192E08, D_80192E0A, D_80192E18, D_80192E1A, D_80192E28, D_80192E2A, D_80192E38, D_80192E3A, D_80192E48, D_80192E4A, D_80192EB8, D_80192EBA, D_80192EC8, D_80192ECA;
extern int D_80192E1C;
extern int D_800B0E64;
extern void func_800C2B40();
extern int func_8006E498();
extern int *func_800C2B28();
extern int func_8006DC18();

void func_8018F240(void *a0, int a1, void *a2)
{
    void *x;
    int t;
    void *y;

    func_800C2B40(a2);
    t = func_8006E498(D_800B0E64, 0x118704);
    x = P(a0, 8);
    P(a2, 0) = x;
    y = P(x, 0x238);
    D_80192E1C = t;
    *(S32 *)((char *)a2 + 4) = *(S32 *)y;
    *(S32 *)((char *)a2 + 0x24) = *(S32 *)((char *)P(P(a2, 0), 0x238) + 0xA0);
    W(a2, 0x44) = func_8006DC18(9);
    H(a2, 0x4A) = 0;
    H(a2, 0x4C) = 0;
    H(a2, 0x48) = 0x96;
    H(a2, 0x4E) = *func_800C2B28(0);
    W(a2, 0x50) = *func_800C2B28(1);
    W(a2, 0x54) = *func_800C2B28(2);
    W(a2, 0x58) = *func_800C2B28(3);
//P<
    D_80192E04 = 0x22;
    D_80192E05 = 0x30;
    D_80192E24 = 0x40;
    D_80192E25 = 0x5;
    D_80192E18 = 0x64;
    D_80192EB4 = 0x6;
    D_80192E08 = 0x0;
    D_80192E0A = 0x80;
    D_80192E00 = 0x80;
    D_80192E01 = 0x80;
    D_80192E02 = 0x80;
    D_80192E06 = 0x0;
    D_80192E28 = 0x0;
    D_80192E2A = 0x80;
    D_80192E20 = 0x80;
    D_80192E21 = 0x80;
    D_80192E22 = 0x80;
    D_80192E26 = 0x0;
    D_80192EC8 = 0x0;
    D_80192ECA = 0x80;
    D_80192EC0 = 0x80;
    D_80192EC1 = 0x80;
    D_80192EC2 = 0x80;
    D_80192EC6 = 0x0;
    D_80192E14 = 0x68;
    D_80192E15 = 0x7;
    D_80192E1A = 0x80;
    D_80192E10 = 0x80;
    D_80192E11 = 0x80;
    D_80192E12 = 0x80;
    D_80192E16 = 0x0;
    D_80192EB5 = 0x60;
    D_80192E44 = 0x2;
    D_80192EB8 = 0x0;
    D_80192EBA = 0x60;
    D_80192EB0 = 0x80;
    D_80192EB1 = 0x80;
    D_80192EB2 = 0x80;
    D_80192EB6 = 0x0;
    D_80192E45 = 0x20;
    D_80192E48 = 0x0;
    D_80192E4A = 0x60;
    D_80192E40 = 0x80;
    D_80192E41 = 0x80;
    D_80192E42 = 0x80;
    D_80192E46 = 0x0;
    D_80192E34 = 0x68;
    D_80192E35 = 0x7;
    D_80192E38 = 0x0;
    D_80192E3A = 0x80;
    D_80192E36 = 0x0;
//P>
}
