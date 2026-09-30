extern int D_800E27EC;
extern char D_800E1FEC;
extern short D_800E2234;
extern short D_800E2236;
extern short D_800E2238;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80071A54(void);
extern int func_80077AA4(int a0, int a1);
extern int func_80077CF4(int a0);
extern void func_800783E4();
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800CEE20(short *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, int *a8);

int func_800DD380(int a0, short *a1)
{
    int blk[2];
    short vec[4];
    short pos[4];
    int s0v;
    int s2v;
    int r;
    int w;
    int idx;
    int t;
    int u;

    switch (a0) {
    case 1:
        a1[5] += 1;
        if (a1[4] < 2) {
            break;
        }
        return 1;
    case 2:
        s2v = 0;
        switch (a1[4]) {
        case 0:
            s0v = (a1[5] << 12) / 52;
            func_800783E4(a1, &D_800E2234, 0x1000 - s0v, s0v, pos);
            pos[0] += func_80077CF4(a1[7] + D_800E27EC * 160) / 16;
            a1[1] -= 2;
            func_800CF3AC(&D_800E1FEC, blk, (D_800E27EC << 6) / 52);
            s2v = func_80077CF4(D_800E27EC << 7) / 2 + 0x1000;
            if (a1[5] >= 0x34) {
                a1[4] = 1;
                a1[5] = 0;
                a1[6] = 0;
            }
            break;
        case 1:
            pos[0] = a1[0];
            pos[1] = a1[1];
            pos[2] = a1[2];
            s2v = func_80077CF4((D_800E27EC << 11) / 12);
            s0v = s2v / 32;
            r = func_80071A54();
            u = *(short *)((char *)a1 + 10);
            blk[0] = s0v | (s0v << 8) | ((r & 0xF) << 16);
            if (u >= 0xC) {
                *(short *)((char *)a1 + 12) += 1;
                if (*(short *)((char *)a1 + 12) < 4) {
                    *a1 = D_800E2234;
                    *(short *)((char *)a1 + 2) = D_800E2236;
                    *(short *)((char *)a1 + 4) = D_800E2238;
                    *a1 += func_80071A54() % 400 - 200;
                    *(short *)((char *)a1 + 2) += func_80071A54() % 400 - 200;
                    *(short *)((char *)a1 + 4) += func_80071A54() % 400 - 200;
                    *(short *)((char *)a1 + 10) = 0;
                } else {
                    *(short *)((char *)a1 + 8) = 2;
                }
            }
            break;
        }
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = D_800E27EC * 12;
        vec[3] = 0;
        idx = D_800F336C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0x20, w);
        func_800CEE20(pos, vec, s2v, s2v, 0x80, t & 0xFFFF, 1, 0x80, blk);
        break;
    default:
        return 0;
    }
    return 0;
}
