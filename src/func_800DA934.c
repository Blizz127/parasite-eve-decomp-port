extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern unsigned short D_800942EC;
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
extern void func_800DA780();

int func_800DA934(int a0, unsigned char *a1)
{
    short *p;
    unsigned char *q;
    unsigned char *r;
    int ix;
    int av;

    switch (a0) {
    case 0:
        q = D_800F32D0;
        *(short *)(a1 + 0) = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x268);
        *(short *)(a1 + 2) = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x26A);
        r = D_800F33E0;
        *(short *)(a1 + 4) = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x26C);
        *(short *)(a1 + 2) = D_800942EC;
        return func_800CE560(*(void **)(r + 8), 8, 0xC, func_800DA780);
    case 1:
        if (D_800E27EC < 0x28 && (D_800E27EC & 1)) {
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                p[0] = *(unsigned short *)(a1 + 0) + func_80071A54() % 400 - 200;
                p[1] = *(unsigned short *)(a1 + 2) - 0x320;
                p[2] = *(unsigned short *)(a1 + 4) + func_80071A54() % 400 - 200;
                p[3] = (func_80071A54() & 7) + 0x2A;
            }
        }
        if (D_800E27EC < 0x46) {
            break;
        }
        return 1;
    case 2:
        ix = D_800E11E6;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        av = *(unsigned short *)((char *)D_800E2850 + ix * 2);
        D_800F336C = 1;
        D_800F3370 = av;
        func_800CEDA8(1);
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0x20;
        break;
    default:
        return 0;
    }
    return 0;
}
