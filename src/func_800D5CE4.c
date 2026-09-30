extern int D_800E27EC;
extern char D_800E1694;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80071A54(void);
extern int func_80077CF4(int a0);
extern int func_80077AA4(int a0, int a1);
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800CEE20(short *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, int *a8);

int func_800D5CE4(int a0, unsigned short *a1)
{
    short buf[4];
    short vec[4];
    int blk[2];
    int s0v;
    int w;
    int idx;
    int t;
    int r1;
    int r2;

    switch (a0) {
    case 1:
        a1[0] = a1[0] + a1[3];
        a1[1] = a1[1] + a1[4];
        a1[2] = a1[2] + a1[5];
        r1 = func_80071A54() & 1;
        r2 = a1[4] + 2;
        a1[4] = r2 + r1;
        if (D_800E27EC < 0x18) {
            break;
        }
        return 1;
    case 2:
        func_800CF3AC(&D_800E1694, blk, D_800E27EC);
        buf[0] = a1[0];
        buf[1] = a1[1];
        buf[2] = a1[2];
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = a1[6] + D_800E27EC * 32;
        s0v = func_80077CF4((D_800E27EC << 10) / 24) + 0x1000;
        idx = D_800F336C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0x20, w);
        func_800CEE20(buf, vec, s0v, s0v, D_800F336A * (D_800E27EC / 3) + 0x80, t & 0xFFFF, 1, 0x80, blk);
        break;
    default:
        return 0;
    }
    return 0;
}
