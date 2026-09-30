extern int D_800E27EC;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern unsigned short D_800E11E4;
extern unsigned short D_800E2850[];
extern short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F336E;
extern short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80071A54(void);
extern int func_80077AA4(int a0, int a1);
extern int func_80077CF4(int a0);
extern int func_80077DC4(int a0);
extern int func_8006DE80(int a0, int a1, short a2, short a3, short a4);
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CEDA8();
extern void func_800CEE20(short *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8);
extern void func_800D5CE4();

int func_800D5EB4(int a0, short *a1)
{
    short vec[4];
    short *p;
    unsigned char *q;
    int t;
    int s0v;
    int s2v;
    int s2b;
    int ix;
    int av;
    int w;
    int idx;
    int u;
    int k;
    int av2;

    switch (a0) {
    case 0:
        t = func_80071A54();
        q = D_800F32D0;
        *(int *)(a1 + 4) = t;
        a1[0] = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x268);
        a1[1] = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x26A);
        a1[2] = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x26C);
        func_8006DE80(0x487, 0, a1[0], a1[1], a1[2]);
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x10, 8, func_800D5CE4);
    case 1:
        if (D_800E27EC < 8) {
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                p[0] = a1[0] + (func_80071A54() & 0x7F) - 0x40;
                p[1] = a1[1] + (func_80071A54() & 0xFF) - 0x80;
                p[2] = a1[2] + (func_80071A54() & 0x7F) - 0x40;
                s2v = (func_80071A54() & 7) + 0x10;
                p[3] = func_80077CF4(*(int *)(a1 + 4)) * s2v / 4096;
                p[5] = func_80077DC4(*(int *)(a1 + 4)) * s2v / 4096;
                p[4] = -(func_80071A54() & 7) - 0xC;
                p[6] = func_80071A54();
                *(int *)(a1 + 4) += 0x955;
            }
        }
        if (D_800E27EC < 0x1E) {
            break;
        }
        return 1;
    case 2:
        ix = D_800E11E4;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        av = *(unsigned short *)((char *)D_800E2850 + ix * 2);
        D_800F336C = 0;
        D_800F3370 = av;
        func_800CEDA8(0);
        u = D_800E27EC;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0x40;
        *(short *)((char *)a1 + 2) -= 4;
        s0v = func_80077DC4((u << 10) / 30) / 32;
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = -0x200;
        vec[3] = 0;
        idx = D_800F336C;
        w = D_800E1204[idx];
        s2b = 0x2400;
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0x20, w);
        func_800CEE20(a1, vec, s2b, s2b, 0x6C, t & 0xFFFF, 1, s0v, 0);
        k = D_800E11E4;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        av2 = *(unsigned short *)((char *)D_800E2850 + k * 2);
        D_800F336C = 0;
        D_800F3370 = av2;
        func_800CEDA8(0);
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0x40;
        break;
    default:
        return 0;
    }
    return 0;
}
