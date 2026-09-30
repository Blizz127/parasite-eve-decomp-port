typedef struct {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
} B4;

extern B4 D_800C22BC;
extern int D_800E27EC;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern unsigned char *D_8009D254;
extern unsigned short D_800E11E6;
extern unsigned short D_800E11F6;
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
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CEAE8();
extern void func_800CEDA8();
extern void func_800CEE20();
extern void func_800D004C();
extern void func_800D0728();
extern void func_800DE7A8();

int func_800DEA30(int a0, short *a1)
{
    short vec[4];
    short v[4];
    B4 blk;
    short *p;
    short *f;
    unsigned char *q;
    int t;
    int T;
    int i;
    int s0v;
    int s2v;
    int ix;
    int av;
    int w;
    int idx;
    int T2;
    int ix2;
    int av2;

    blk = D_800C22BC;
    switch (a0) {
    case 0:
        t = func_80071A54();
        q = D_800F32D0;
        *(int *)(a1 + 4) = t;
        a1[0] = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x268);
        a1[1] = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x26A);
        a1[2] = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x26C);
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x10, 0x20, func_800DE7A8);
    case 1:
        if (D_800E27EC < 0x10) {
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                i = 0;
                do {
                    i++;
                    v[0] = func_80071A54() % 48 - 24;
                    v[2] = -(func_80071A54() % 80);
                    v[1] = func_80071A54() % 48 - 32;
                    func_800CEAE8(*(void **)(D_8009D254 + 0x238), v, v);
                    p[3] = v[0];
                    p[4] = v[1];
                    p[5] = v[2];
                    p[0] = a1[0] + (func_80071A54() & 0xFF) - 0x80;
                    p[1] = a1[1] + (func_80071A54() & 0xFF) - 0x80;
                    p[2] = a1[2] + (func_80071A54() & 0xFF) - 0x80;
                    p[7] = (func_80071A54() & 0x3F) + 0x50;
                    p[6] = func_80071A54();
                    *(int *)(a1 + 4) += 0x955;
                } while (i < 2);
            }
        }
        if (D_800E27EC < 0x30) {
            break;
        }
        return 1;
    case 2:
        ix = D_800E11F6;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        av = *(unsigned short *)((char *)D_800E2850 + ix * 2);
        D_800F336C = 1;
        D_800F3370 = av;
        func_800CEDA8(1, ix * 2);
        T = D_800E27EC;
        D_800F3372 = 5;
        D_800F336E = 1;
        D_800F3374 = 0x40;
        if (T < 0x19) {
            a1[1] -= 6;
            s2v = func_80077DC4((T << 10) / 24) * 240 / 4096;
            vec[0] = 0;
            vec[1] = 0;
            vec[2] = D_800E27EC << 4;
            vec[3] = 0;
            s0v = 0x3000 - func_80077DC4((D_800E27EC << 10) / 12);
            idx = D_800F336C;
            w = D_800E1204[idx] + ((idx == 4 && D_800F3428 != 0) ? 8 : 4);
            t = func_80077AA4(0, w);
            func_800CEE20(a1, vec, s0v, s0v, 0x40, t & 0xFFFF, 1, s2v, 0);
            if (D_800E27EC < 9) {
                s2v = func_80077DC4(D_800E27EC << 7) / 32;
                func_800D004C(a1, 100, 700, 0x10, vec, s0v, s0v, &blk, 0, s2v, 1);
            }
        }
        T2 = D_800E27EC;
        f = &D_800F3374;
        *f = 0;
        if (T2 < 0x11) {
            s2v = 0x80 - (T2 - 4) * 8;
            v[0] = a1[0];
            v[1] = a1[1];
            v[2] = a1[2];
            vec[0] = 0;
            vec[1] = 0;
            vec[2] = T2 << 7;
            s0v = func_80077CF4((T2 - 4) << 6) * 2;
            func_800D0728(v, 600, 700, 0x18, vec, s0v, s0v, 0, &blk, s2v, 1);
        }
        ix2 = D_800E11E6;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        av2 = *(unsigned short *)((char *)D_800E2850 + ix2 * 2);
        D_800F336C = 1;
        D_800F3370 = av2;
        func_800CEDA8(1, ix2 * 2);
        D_800F336E = 0;
        D_800F3372 = 0;
        *f = 0x18;
        break;
    default:
        return 0;
    }
    return 0;
}
