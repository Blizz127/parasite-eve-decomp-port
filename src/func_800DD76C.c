extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern short D_800E2234;
extern short D_800E2236;
extern short D_800E2238;
extern void *D_8009D254;
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
extern int func_80071A54();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CE870();
extern void func_800CEDA8();
extern void func_800DD380();

int func_800DD76C(int a0, short *a1)
{
    short *p;
    int ix;
    int av;

    switch (a0) {
    case 0:
        func_800CE870(D_8009D254, 1, a1);
        a1[4] = a1[0];
        a1[5] = a1[1];
        a1[6] = a1[2];
        a1[5] = a1[5] - 0x226;
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x10, 0x18, func_800DD380);
    case 1:
        if (D_800E27EC < 0x31 && (D_800E27EC & 1)) {
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                p[0] = a1[0] + func_80071A54() % 700 - 350;
                p[1] = a1[1];
                p[2] = a1[2] + func_80071A54() % 700 - 350;
                p[4] = 0;
                p[5] = 0;
                p[7] = func_80071A54();
            }
        }
        if (D_800E27EC < 0x8C) {
            break;
        }
        return 1;
    case 2:
        D_800E2234 = *(short *)((char *)a1 + 8);
        D_800E2236 = *(short *)((char *)a1 + 0xA);
        D_800E2238 = *(short *)((char *)a1 + 0xC);
        ix = D_800E11E6;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        av = *(unsigned short *)((char *)D_800E2850 + ix * 2);
        D_800F336C = 1;
        D_800F3370 = av;
        func_800CEDA8(1, ix * 2);
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0x10;
        break;
    default:
        return 0;
    }
    return 0;
}
