/* room_m0174i — func_80190358, blob offset 0x1370, 0x320 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * Object init + 7 colour-block global groups; first draft exact in retail store order once the func_8006E498 result store follows the x/y pointer loads. */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

typedef struct { int w[8]; } S32;
extern unsigned char D_801973E8, D_801973E9, D_801973EA, D_801973EC, D_801973ED, D_801973EE, D_801973F8, D_801973F9, D_801973FA, D_801973FC, D_801973FD, D_801973FE, D_80197410, D_80197411, D_80197412, D_80197414, D_80197415, D_80197416, D_80197424, D_80197425, D_80197426, D_80197440, D_80197441, D_80197442, D_80197444, D_80197445, D_80197446, D_801974C0, D_801974C1, D_801974C2, D_801974C4, D_801974C5, D_801974C6, D_801974D0, D_801974D1, D_801974D2, D_801974D6;
extern short D_801973F0, D_801973F2, D_80197400, D_80197402, D_80197418, D_8019741A, D_80197428, D_8019742A, D_80197448, D_8019744A, D_801974C8, D_801974CA, D_801974D8, D_801974DA;
extern int D_80197404;
extern int D_800B0E64;
extern void func_800C2B40();
extern int func_8006E498();
extern int *func_800C2B28();
extern int func_8006DC18();

void func_80190358(void *a0, int a1, void *a2)
{
    void *x;
    int t;
    void *y;

    func_800C2B40(a2);
    t = func_8006E498(D_800B0E64, 0x118704);
    x = P(a0, 8);
    P(a2, 0) = x;
    y = P(x, 0x238);
    D_80197404 = t;
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
    D_801973EC = 0x22;
    D_801973ED = 0x30;
    D_80197414 = 0x40;
    D_80197415 = 0x5;
    D_80197400 = 0x64;
    D_801974C4 = 0x6;
    D_801973F0 = 0x0;
    D_801973F2 = 0x80;
    D_801973E8 = 0x80;
    D_801973E9 = 0x80;
    D_801973EA = 0x80;
    D_801973EE = 0x0;
    D_80197418 = 0x0;
    D_8019741A = 0x80;
    D_80197410 = 0x80;
    D_80197411 = 0x80;
    D_80197412 = 0x80;
    D_80197416 = 0x0;
    D_801974D8 = 0x0;
    D_801974DA = 0x80;
    D_801974D0 = 0x80;
    D_801974D1 = 0x80;
    D_801974D2 = 0x80;
    D_801974D6 = 0x0;
    D_801973FC = 0x68;
    D_801973FD = 0x7;
    D_80197402 = 0x80;
    D_801973F8 = 0x80;
    D_801973F9 = 0x80;
    D_801973FA = 0x80;
    D_801973FE = 0x0;
    D_801974C5 = 0x60;
    D_80197444 = 0x2;
    D_801974C8 = 0x0;
    D_801974CA = 0x60;
    D_801974C0 = 0x80;
    D_801974C1 = 0x80;
    D_801974C2 = 0x80;
    D_801974C6 = 0x0;
    D_80197445 = 0x20;
    D_80197448 = 0x0;
    D_8019744A = 0x60;
    D_80197440 = 0x80;
    D_80197441 = 0x80;
    D_80197442 = 0x80;
    D_80197446 = 0x0;
    D_80197424 = 0x68;
    D_80197425 = 0x7;
    D_80197428 = 0x0;
    D_8019742A = 0x80;
    D_80197426 = 0x0;
//P>
}
