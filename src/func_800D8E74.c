typedef struct {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
} B4;

extern B4 D_800C22E0;
extern int D_800E27EC;
extern unsigned short D_800E2200;
extern unsigned short D_800E2202;
extern unsigned short D_800E2204;
extern void *D_800E2208;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80071A54(void);
extern int func_80077AA4(int a0, int a1);
extern int func_80077CF4(int a0);
extern int func_80077DC4(int a0);
extern short *func_800CE610();
extern void func_800CEE20(short *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, B4 *a8);

int func_800D8E74(int a0, short *a1)
{
    short pos[4];
    short vec[4];
    B4 blk;
    short *p;
    int s1v;
    int v;
    int w;
    int idx;
    int t;

    blk = D_800C22E0;
    switch (a0) {
    case 1:
        a1[3] = D_800E2200 + func_80077DC4(a1[1]) * a1[6] / 4096;
        a1[5] = D_800E2204 + func_80077CF4(a1[1]) * a1[6] / 4096;
        if (a1[7] != 0) {
            a1[4] = D_800E2202 - D_800E27EC * 500 / 36;
        } else {
            v = D_800E27EC * 500 / 36 - 1000;
            a1[4] = D_800E2202 + v;
        }
        a1[1] += 0x60;
        a1[6] = func_80077DC4((D_800E27EC << 10) / 36) * 700 / 4096;
        if (!(D_800E27EC & 1)) {
            p = func_800CE610(D_800E2208);
            if (p != 0) {
                p[0] = a1[3] + (func_80071A54() & 7) - 3;
                p[1] = a1[4] + (func_80071A54() & 7) - 3;
                p[2] = a1[5] + (func_80071A54() & 7) - 3;
            }
        }
        a1[2] += 1;
        if (D_800E27EC < 0x24) {
            break;
        }
        return 1;
    case 2:
        switch (a1[0]) {
        case 0:
            func_80077CF4((a1[2] << 10) / 12);
            s1v = 0x1000;
            if (a1[2] >= 0xC) {
                a1[2] = 0;
                a1[0] = 1;
            }
            break;
        case 1:
            s1v = 0x1000;
            if (a1[2] >= 0xC) {
                a1[2] = 0;
                a1[0] = 2;
            }
            break;
        default:
            s1v = func_80077DC4((a1[2] << 10) / 12);
            a1[6] += a1[2] * 4;
            break;
        }
        pos[0] = a1[3];
        pos[1] = a1[4];
        pos[2] = a1[5];
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = D_800E27EC << 7;
        vec[3] = 0;
        s1v <<= 1;
        idx = D_800F336C;
        w = D_800E1204[idx] + ((idx == 4 && D_800F3428 != 0) ? 6 : 2);
        t = func_80077AA4(0, w);
        func_800CEE20(pos, vec, s1v, s1v, 0xE4, t & 0xFFFF, 1, 0x80, &blk);
        break;
    default:
        return 0;
    }
    return 0;
}
