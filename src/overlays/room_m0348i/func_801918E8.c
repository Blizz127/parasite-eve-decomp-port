/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_801918E8 — blob offset 0x2900, 0x1b0 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80192A00; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

typedef struct { int w[8]; } S32;
extern short D_80192E8C;
extern short D_80192E92, D_80192E94, D_80192E8E, D_80192E90;
extern unsigned char D_80192E88, D_80192E89, D_80192E8A, D_80192E84, D_80192E85, D_80192E86;
extern short D_80192EA4, D_80192EAA, D_80192EAC, D_80192EA8, D_80192EA6;
extern unsigned char D_80192EA0, D_80192EA1, D_80192EA2, D_80192E9C, D_80192E9D, D_80192E9E;
extern char D_80192C00[];
extern char **func_800C2B50();
extern void func_800C4E50();

void func_801918E8(int a0, int a1, char *a2)
{
    char **x;
    short *b;
    char *p;

    x = func_800C2B50();
    asm volatile("" : "=r"(b) : "0"(&D_80192E8C));
    *(S32 *)a2 = *(S32 *)(*(char **)(*x + 0x238) + 0xA0);
    *(short *)(a2 + 0x30) = 0x4B0;
    *(short *)(a2 + 0x34) = 0xFF;
    p = D_80192C00;
    *b = 0x10;
    D_80192E92 = -0x1F4;
    D_80192E94 = 0x80;
    D_80192E88 = 0xFA;
    D_80192E89 = 0xFA;
    D_80192E8A = 0x78;
    D_80192E84 = 0;
    D_80192E85 = 0;
    D_80192E86 = 0;
    D_80192E8E = 0xC80;
    D_80192E90 = 0x578;
    *(char **)((char *)b - 0xC) = p;
    func_800C4E50((char *)b - 0xC);
    D_80192EA4 = 0x10;
    D_80192EAA = -0x1F4;
    D_80192EAC = 0x80;
    D_80192EA0 = 0xFF;
    D_80192EA1 = 0xFF;
    D_80192EA2 = 0xFF;
    D_80192E9C = 0xFA;
    D_80192E9D = 0xFA;
    D_80192E9E = 0x78;
    D_80192EA8 = 0;
    D_80192EA6 = 0x578;
    *(char **)((char *)b + 0xC) = p + 0x100;
    func_800C4E50((char *)b + 0xC);
}
