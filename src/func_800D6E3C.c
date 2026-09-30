extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern unsigned char *D_800F32D0;
extern unsigned short D_800E11E4;
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
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CEDA8();
extern void func_800D6C58();

int func_800D6E3C(int a0, int *a1)
{
    short buf[4];
    short *p;
    unsigned char *q;
    unsigned char *o;
    int i;
    int s1v;
    int t;
    int r1;
    int r2;
    int av;
    int ix;

    switch (a0) {
    case 0:
        t = func_80071A54();
        q = D_800F33E0;
        *a1 = t;
        return func_800CE560(*(void **)(q + 8), 0x10, 0x18, func_800D6C58);
    case 1:
        if (D_800E27EC < 8) {
            for (i = 0; i < 3; i++) {
                p = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (p != 0) {
                    o = D_800F32D0;
                    buf[0] = *(unsigned short *)(*(unsigned char **)(o + 8) + 0x268);
                    buf[1] = *(unsigned short *)(*(unsigned char **)(o + 8) + 0x26A);
                    buf[2] = *(unsigned short *)(*(unsigned char **)(o + 8) + 0x26C);
                    p[0] = buf[0];
                    p[1] = buf[1];
                    p[2] = buf[2];
                    r1 = func_80071A54() & 0x1F;
                    r2 = *a1 + 0x955;
                    *a1 = r2 + r1;
                    s1v = (func_80071A54() & 0x7F) + 0x46;
                    p[3] = func_80077CF4(*a1) * s1v / 4096;
                    p[5] = func_80077DC4(*a1) * s1v / 4096;
                    p[4] = -(func_80071A54() & 0x1F) - 0x24;
                    p[6] = (func_80071A54() & 7) + 0x1A;
                    p[7] = func_80071A54();
                }
            }
        }
        if (D_800E27EC < 0x28) {
            break;
        }
        return 1;
    case 2:
        ix = D_800E11E4;
        D_800F3368 = 0x10;
        D_800F336A = 1;
        D_800F3376 = 0x10;
        D_800F3378 = 0x10;
        av = *(unsigned short *)((char *)D_800E2850 + ix * 2);
        D_800F336C = 0;
        D_800F3370 = av;
        func_800CEDA8(0);
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0;
        break;
    default:
        return 0;
    }
    return 0;
}
