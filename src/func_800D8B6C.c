extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern void *D_8009D254;
extern unsigned short D_800E11F6;
extern unsigned short D_800E2850[];
extern short D_800E21F8[];
extern short D_800F3368;
extern short D_800F336A;
extern short D_800F336C;
extern short D_800F336E;
extern short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern int func_80071A54();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CE870();
extern void func_800CEDA8();
extern void func_800D8978();

int func_800D8B6C(int a0, unsigned char *a1)
{
    short buf[4];
    short *p;
    int ix;
    int av;
    int t;
    void *q;

    switch (a0) {
    case 0:
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0xC, 0xE, func_800D8978);
    case 1:
        if (D_800E27EC < 0x20 && (D_800E27EC & 1)) {
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                t = func_80071A54();
                q = D_8009D254;
                p[1] = t;
                p[0] = 0;
                p[2] = 0;
                func_800CE870(q, 1, buf);
                p[4] = buf[1] - 0x200;
            }
        }
        if (D_800E27EC >= 0x32) {
            return 1;
        }
    case 2:
        func_800CE870(D_8009D254, 1, D_800E21F8);
        ix = D_800E11F6;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        av = *(unsigned short *)((char *)D_800E2850 + ix * 2);
        D_800F336C = 1;
        D_800F3370 = av;
        func_800CEDA8(1, ix * 2);
        D_800F336E = 1;
        D_800F3372 = 0;
        D_800F3374 = 8;
        break;
    default:
        return 0;
    }
    return 0;
}
