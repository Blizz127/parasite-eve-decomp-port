extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern unsigned char *D_800E2368;
extern void *D_8009D254;
extern unsigned short D_800E11E8;
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
extern void func_800CE870();
extern void func_800D4928();

int func_800D4C24(int a0, int *a1)
{
    short buf[4];
    short *p;
    int i;
    int s1v;
    int t;
    unsigned char *q;
    unsigned char *q2;
    int av;
    register int h asm("$4");
    register int g asm("$2");
    int ix;

    switch (a0) {
    case 0:
        t = func_80071A54();
        q = D_800F33E0;
        *a1 = t;
        return func_800CE560(*(void **)(q + 8), 0x10, 0x10, func_800D4928);
    case 1:
        if (D_800E27EC < 8) {
            for (i = 0; i <= 0; i++) {
                p = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (p != 0) {
                    func_800CE870(D_8009D254, 1, buf);
                    q2 = D_800E2368;
                    p[0] = buf[0];
                    if (*(short *)(q2 + 0x1E) == 0) {
                        p[1] = buf[1] - 0x258;
                    } else {
                        p[1] = buf[1] - 0x190;
                    }
                    p[2] = buf[2];
                    s1v = (func_80071A54() & 7) + 0x20;
                    p[3] = func_80077CF4(*a1) * s1v / 4096;
                    p[5] = func_80077DC4(*a1) * s1v / 4096;
                    p[4] = -(func_80071A54() & 7) - 0xC;
                    p[6] = (func_80071A54() & 3) + 0x1A;
                    p[7] = func_80071A54();
                    *a1 = *a1 + 0x200;
                }
            }
        }
        if (D_800E27EC < 0x28) {
            break;
        }
        return 1;
    case 2:
        ix = D_800E11E8;
        h = 0x10;
        g = 1;
        D_800F336A = g;
        D_800F3368 = h;
        D_800F3376 = h;
        D_800F3378 = h;
        g = 0x20;
        D_800F3376 = g;
        D_800F3378 = h;
        av = *(unsigned short *)((char *)D_800E2850 + ix * 2);
        g = 2;
        D_800F336C = g;
        D_800F336E = 0;
        D_800F3372 = 0;
        g = 0x20;
        D_800F3374 = g;
        D_800F3370 = av;
        break;
    default:
        return 0;
    }
    return 0;
}
