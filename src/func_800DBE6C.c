extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
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
extern void func_800DBCD8();

int func_800DBE6C(int a0, unsigned char *a1)
{
    short *p;
    unsigned char *q;
    int ix;
    int av;

    switch (a0) {
    case 0:
        q = D_800F32D0;
        ((short *)a1)[0] = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x268);
        ((short *)a1)[1] = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x26A);
        ((short *)a1)[2] = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x26C);
        return func_800CE560(*(void **)(D_800F33E0 + 8), 8, 0x14, func_800DBCD8);
    case 1:
        if (D_800E27EC < 0x15) {
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                p[0] = *(unsigned short *)(a1 + 0) + (func_80071A54() & 0x1FF) - 0x100;
                p[1] = *(unsigned short *)(a1 + 2) + (func_80071A54() & 0x1FF) - 0x100;
                p[2] = *(unsigned short *)(a1 + 4) + (func_80071A54() & 0x1FF) - 0x100;
            }
        }
        if (D_800E27EC < 0x35) {
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
