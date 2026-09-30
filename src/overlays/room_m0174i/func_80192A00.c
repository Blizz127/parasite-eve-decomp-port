/* room_m0174i — func_80192A00, blob offset 0x3A18, 0x1B0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * Two sprite-record inits: callee-saved base from asm volatile("" : "=r"(b) : "0"(&D_8019749C)) after func_800C2B50 (retail s2 anchor on [0].hC); other fields absolute externs; S32 copy (ovl5 park 40w -> 0). */

typedef struct { int w[8]; } S32;
extern short D_8019749C;
extern short D_801974A2, D_801974A4, D_8019749E, D_801974A0;
extern unsigned char D_80197498, D_80197499, D_8019749A, D_80197494, D_80197495, D_80197496;
extern short D_801974B4, D_801974BA, D_801974BC, D_801974B8, D_801974B6;
extern unsigned char D_801974B0, D_801974B1, D_801974B2, D_801974AC, D_801974AD, D_801974AE;
extern char D_801971E8[];
extern char **func_800C2B50();
extern void func_800C4E50();

void func_80192A00(int a0, int a1, char *a2)
{
    char **x;
    short *b;
    char *p;

    x = func_800C2B50();
    asm volatile("" : "=r"(b) : "0"(&D_8019749C));
    *(S32 *)a2 = *(S32 *)(*(char **)(*x + 0x238) + 0xA0);
    *(short *)(a2 + 0x30) = 0x4B0;
    *(short *)(a2 + 0x34) = 0xFF;
    p = D_801971E8;
    *b = 0x10;
    D_801974A2 = -0x1F4;
    D_801974A4 = 0x80;
    D_80197498 = 0xFA;
    D_80197499 = 0xFA;
    D_8019749A = 0x78;
    D_80197494 = 0;
    D_80197495 = 0;
    D_80197496 = 0;
    D_8019749E = 0xC80;
    D_801974A0 = 0x578;
    *(char **)((char *)b - 0xC) = p;
    func_800C4E50((char *)b - 0xC);
    D_801974B4 = 0x10;
    D_801974BA = -0x1F4;
    D_801974BC = 0x80;
    D_801974B0 = 0xFF;
    D_801974B1 = 0xFF;
    D_801974B2 = 0xFF;
    D_801974AC = 0xFA;
    D_801974AD = 0xFA;
    D_801974AE = 0x78;
    D_801974B8 = 0;
    D_801974B6 = 0x578;
    *(char **)((char *)b + 0xC) = p + 0x100;
    func_800C4E50((char *)b + 0xC);
}
