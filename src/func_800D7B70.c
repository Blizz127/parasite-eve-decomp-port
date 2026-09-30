extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern void *D_8009D254;
extern char D_800E18F0;
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
extern int func_80077CF4(int a0);
extern int func_80077DC4(int a0);
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CE870();
extern void func_800CEDA8();
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800D0728();
extern void func_800D7A1C();

int func_800D7B70(int a0, short *a1)
{
    int blk[2];
    short pos[4];
    short vec[4];
    short *p;
    void *q;
    int t;
    int T;
    int s0v;
    int ix;
    int av;
    int u;

    switch (a0) {
    case 0:
        t = func_80071A54();
        q = D_8009D254;
        *(int *)(a1 + 4) = t;
        *(int *)(a1 + 6) = 0;
        func_800CE870(q, 0, a1);
        return func_800CE560(*(void **)(D_800F33E0 + 8), 8, 0x18, func_800D7A1C);
    case 1:
        if (D_800E27EC < 0x33) {
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                p[0] = a1[0] + func_80077DC4(*(int *)(a1 + 4)) * *(int *)(a1 + 6) / 4096;
                p[2] = a1[2] + func_80077CF4(*(int *)(a1 + 4)) * *(int *)(a1 + 6) / 4096;
                p[1] = a1[1];
                p[3] = func_80071A54() & 3;
                *(int *)(a1 + 4) += (func_80071A54() & 0x1F) + 0x8AA;
            }
        }
        if (D_800E27EC < 0x4A) {
            break;
        }
        return 1;
    case 2:
        T = D_800E27EC;
        D_800F3374 = 8;
        if (T < 0x33) {
            pos[0] = a1[0];
            pos[1] = a1[1];
            pos[2] = a1[2];
            vec[0] = 0x400;
            vec[1] = 0;
            vec[2] = T << 5;
            vec[3] = 1;
            s0v = func_80077CF4((T << 10) / 50);
            t = s0v / 8;
            u = D_800E27EC;
            *(int *)(a1 + 6) = t;
            func_800CF3AC(&D_800E18F0, blk, u);
            func_800D0728(pos, 350, 500, 20, vec, s0v, s0v, 0, blk, 0x80, 1);
        }
        ix = D_800E11E6;
        D_800F3368 = 0x10;
        D_800F336A = 1;
        D_800F3376 = 0x10;
        D_800F3378 = 0x10;
        av = *(unsigned short *)((char *)D_800E2850 + ix * 2);
        D_800F336C = 1;
        D_800F3370 = av;
        func_800CEDA8(1);
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 4;
        break;
    default:
        return 0;
    }
    return 0;
}
