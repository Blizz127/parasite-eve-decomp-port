extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern unsigned char *D_8009D254;
extern char D_800E20AC;
extern short D_800E223C;
extern short D_800E223E;
extern short D_800E2240;
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
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800D004C();
extern void func_800D0728();
extern void func_800DD9E4();

int func_800DDD70(int a0, short *a1)
{
    short pos[4];
    int blk[2];
    short *p;
    int T;
    int s0v;
    int v;
    int ix;
    int av;

    switch (a0) {
    case 0:
        func_800CE870(D_8009D254, 1, a1);
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x10, 0x20, func_800DD9E4);
    case 1:
        if (D_800E27EC < 0x43) {
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                p[0] = a1[0] + (func_80071A54() & 0x1FF) - 0x100;
                p[1] = a1[1] - func_80071A54() % 600;
                p[2] = a1[2] + (func_80071A54() & 0x1FF) - 0x100;
                p[4] = 0;
                p[5] = 0;
                p[6] = func_80071A54();
            }
        }
        if (D_800E27EC < 0x56) {
            break;
        }
        return 1;
    case 2:
        pos[0] = *(int *)(*(unsigned char **)(D_8009D254 + 0x238) + 0x274);
        pos[1] = *(int *)(*(unsigned char **)(D_8009D254 + 0x238) + 0x278);
        pos[2] = *(int *)(*(unsigned char **)(D_8009D254 + 0x238) + 0x27C);
        T = D_800E27EC;
        D_800E223C = pos[0];
        D_800E223E = pos[1];
        D_800E2240 = pos[2];
        if (T < 0x57) {
            s0v = (T << 12) / 86 + 0x800;
            D_800F3374 = 0x10;
            func_800CF3AC(&D_800E20AC, blk, T);
            v = 1;
            if (D_800E27EC & 1) {
                v = 3;
            }
            func_800D004C(pos, 500, 500, 0x10, 0, s0v, s0v, blk, 0, 0x80, 1);
            ((unsigned char *)blk)[2] = 0;
            func_800D0728(pos, 400, 300, 0x14, 0, s0v, s0v, blk, 0, 0x40, v);
        }
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
        break;
    default:
        return 0;
    }
    return 0;
}
