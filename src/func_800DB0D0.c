extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern void *D_8009D254;
extern unsigned short D_800E221C;
extern unsigned short D_800E221E;
extern unsigned short D_800E2220;
extern short D_800F3374;
extern int func_80071A54();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CE870();
extern void func_800DAF8C();

int func_800DB0D0(int a0, unsigned char *a1)
{
    short *p;
    int t;
    void *q;

    switch (a0) {
    case 0:
        t = func_80071A54();
        q = D_8009D254;
        *(int *)(a1 + 8) = t;
        func_800CE870(q, 0, a1);
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x10, 0x10, func_800DAF8C);
    case 1:
        if (D_800E27EC < 0x11) {
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                p[6] = (func_80071A54() & 0x3FF) - 0x200;
                p[1] = *(int *)(a1 + 8);
                p[2] = func_80071A54();
                p[7] = (func_80071A54() & 0x7F) - 0x40;
                *(int *)(a1 + 8) += 0x8AA + (func_80071A54() & 0x1F);
            }
        }
        if (D_800E27EC < 0x49) {
            break;
        }
        return 1;
    case 2:
        D_800E221C = *(unsigned short *)(a1 + 0);
        D_800E221E = *(unsigned short *)(a1 + 2);
        D_800E2220 = *(unsigned short *)(a1 + 4);
        D_800F3374 = 8;
        break;
    default:
        return 0;
    }
    return 0;
}
