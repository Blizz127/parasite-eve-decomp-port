typedef struct {
    unsigned char b[8];
} B8;

extern B8 D_800C22F0;
extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern void *D_8009D254;
extern char D_800E213C;
extern short D_800E2244;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80071A54(void);
extern int func_80077AA4(int a0, int a1);
extern int func_80077CF4(int a0);
extern int func_80077DC4(int a0);
extern short *func_800CE610();
extern void func_800CE8F0();
extern void func_800CEE20();
extern void func_800CF3AC(char *a0, int *a1, int a2);

int func_800DEFFC(int a0, short *a1)
{
    B8 blk;
    int blk2[2];
    short *p;
    int s0v;
    int s2v;
    int T;
    int w;
    unsigned short idx;
    int t;

    blk = D_800C22F0;
    switch (a0) {
    case 1:
        switch (a1[8]) {
        case 0:
            a1[9] += 1;
            func_800CE8F0(D_8009D254, a1[3], &blk, a1);
            if (D_800E27EC & 3) {
                return 0;
            }
            if (func_80071A54() % 3 == 0) {
                p = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (p == 0) {
                    break;
                }
                p[0] = a1[0];
                p[1] = a1[1];
                p[2] = a1[2];
                p[4] = (func_80071A54() & 7) - 3;
                p[6] = (func_80071A54() & 7) - 3;
                p[5] = (func_80071A54() & 0xF) - 0xC;
                p[8] = 2;
                p[9] = 0;
            } else {
                p = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (p == 0) {
                    return 0;
                }
                p[0] = a1[0];
                p[1] = a1[1];
                p[2] = a1[2];
                p[0] += (func_80071A54() & 0x7F) - 0x40;
                p[1] += (func_80071A54() & 0x7F) - 0x40;
                p[2] += (func_80071A54() & 0x7F) - 0x40;
                p[4] = (func_80071A54() & 0xF) - 7;
                p[6] = (func_80071A54() & 0xF) - 7;
                p[5] = (func_80071A54() & 7) - 0xB;
                p[8] = 1;
                p[9] = 0;
            }
            break;
        case 1:
            a1[9] += 1;
            a1[0] += a1[4];
            a1[1] += a1[5];
            a1[2] += a1[6];
            a1[0] += (func_80071A54() & 7) - 3;
            a1[1] += (func_80071A54() & 7) - 3;
            a1[2] += (func_80071A54() & 7) - 3;
            if (a1[9] < 0x10) {
                break;
            }
            return 1;
        case 2:
            a1[9] += 1;
            a1[0] += a1[4];
            a1[1] += a1[5];
            a1[2] += a1[6];
            if (a1[9] < 0x18) {
                break;
            }
            return 1;
        default:
            return 0;
        }
        break;
    case 2:
        switch (a1[8]) {
        case 0:
            s2v = 0x1800;
            if (a1[9] < 0x21) {
                s2v = func_80077CF4(a1[9] << 5) + 0x800;
            }
            if (a1[9] < 0x19) {
                s0v = func_80077CF4((a1[9] << 10) / 24) / 32;
            } else {
                s0v = 0x64;
                if (a1[9] & 1) {
                    s0v = 0x80;
                }
            }
            if (D_800E2244 < s0v) {
                s0v = D_800E2244;
            }
            idx = D_800F336C;
            w = D_800E1204[idx];
            if (idx == 4 && D_800F3428 != 0) {
                w += 4;
            }
            t = func_80077AA4(0x10, w);
            func_800CEE20(a1, 0, s2v / 2, s2v / 2, D_800F336A * (a1[9] & 3) + 0x68, t & 0xFFFF, 1, s0v, 0);
            break;
        case 1:
            s0v = a1[9] << 6;
            s2v = func_80077DC4(s0v) / 2 + 0x800;
            s0v = func_80077DC4(s0v) / 32;
            if (D_800E2244 < s0v) {
                s0v = D_800E2244;
            }
            idx = D_800F336C;
            w = D_800E1204[idx];
            if (idx == 4 && D_800F3428 != 0) {
                w += 4;
            }
            t = func_80077AA4(0x10, w);
            func_800CEE20(a1, 0, s2v, s2v, D_800F336A * (a1[9] & 3) + 0x68, t & 0xFFFF, 1, s0v / 2, 0);
            break;
        case 2:
            s2v = func_80077CF4((a1[9] << 10) / 24) + 0x1000;
            func_800CF3AC(&D_800E213C, blk2, a1[9]);
            idx = D_800F336C;
            w = D_800E1204[idx];
            if (idx == 4 && D_800F3428 != 0) {
                w += 4;
            }
            t = func_80077AA4(0, w);
            func_800CEE20(a1, 0, s2v / 2, s2v, D_800F336A * (short)(a1[9] / 6) + 0x60, t & 0xFFFF, 1, 0x80, blk2);
            break;
        default:
            return 0;
        }
        break;
    default:
        return 0;
    }
    return 0;
}
