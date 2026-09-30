extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern unsigned char D_800E2164[];
extern unsigned short D_800E11E6;
extern unsigned short D_800E2850[];
extern short D_800F3368;
extern short D_800F336A;
extern short D_800F336C;
extern short D_800F336E;
extern short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern short D_800E2244;
extern int func_800CE560();
extern unsigned char *func_800CE610();
extern void func_800CEDA8();
extern void func_800DEFFC();

int func_800DF6AC(int a0, short *a1)
{
    unsigned char *t;
    int v;
    int u;
    int ix;
    unsigned char *q;
    register int h asm("$2");
    int av;

    switch (a0) {
    case 0:
        q = D_800F33E0;
        h = 0xA0;
        a1[0] = 0;
        a1[1] = h;
        return func_800CE560(*(void **)(q + 8), 0x14, 0x20, func_800DEFFC);
    case 1:
        if ((D_800E27EC & 3) == 0) {
            if (a1[0] < 8) {
                t = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (t != 0) {
                    v = D_800E2164[a1[0]];
                    *(short *)(t + 0x10) = 0;
                    *(short *)(t + 0x12) = 0;
                    *(short *)(t + 6) = v;
                }
                a1[0] = a1[0] + 1;
            }
        }
        if (D_800E27EC >= 0x20) {
            a1[1] = a1[1] - 2;
        }
        if (a1[1] > 0) {
            return 0;
        }
        a1[1] = 0;
        return 1;
    case 2:
        u = *(unsigned short *)(a1 + 1);
        ix = D_800E11E6;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        av = *(unsigned short *)((char *)D_800E2850 + ix * 2);
        D_800E2244 = u;
        D_800F336C = 1;
        D_800F3370 = av;
        func_800CEDA8(1, ix * 2, u);
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0x18;
        break;
    }
    return 0;
}
