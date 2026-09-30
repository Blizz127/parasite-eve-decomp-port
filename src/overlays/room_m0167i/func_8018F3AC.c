/* room_m0167i — func_8018F3AC, blob offset 0x3C4, 0x144 bytes. Profile era_o2_g0 (default).
 * LINK_EXACT at the target VMA (lane ovl3 2026-09-27); global store order is load-bearing. */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

typedef struct { int w[8]; } S32;
extern unsigned char D_801932C0, D_801932C1, D_801932C2, D_801932C4, D_801932C5, D_801932C6;
extern unsigned char D_801932D0, D_801932D1, D_801932D2, D_801932D4, D_801932D5, D_801932D6;
extern short D_801932C8, D_801932CA, D_801932D8, D_801932DA;
extern void func_800C2B40();
extern int func_8006DC18();

void func_8018F3AC(void *a0, int a1, void *a2)
{
    void *x;

    func_800C2B40(a2);
    W(a2, 0x24) = func_8006DC18(0xB);
    x = P(a0, 8);
    P(a2, 0) = x;
    *(S32 *)((char *)a2 + 4) = *(S32 *)P(x, 0x238);
    H(a2, 0x28) = 0x28;
    H(a2, 0x2A) = 0;
    H(a2, 0x2C) = 0;
    D_801932D4 = 4;
    D_801932D5 = 1;
    D_801932DA = 0x80;
    D_801932D8 = 0;
    D_801932D0 = 0x80;
    D_801932C4 = 8;
    D_801932C5 = 2;
    D_801932D1 = 0x80;
    D_801932D2 = 0x80;
    D_801932D6 = 0;
    D_801932C8 = 0;
    D_801932CA = 0x30;
    D_801932C0 = 0x80;
    D_801932C1 = 0x80;
    D_801932C2 = 0x80;
    D_801932C6 = 0;
}
