extern int D_800E27EC;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80077CF4(int a0);
extern int func_80077AA4(int a0, int a1);
extern void func_800CEE20(short *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8);

int func_800DA780(int a0, short *a1)
{
    short buf[4];
    short vec[4];
    int s0v;
    int w;
    int idx;
    int t;

    switch (a0) {
    case 1:
        a1[1] = a1[1] + a1[3];
        if (D_800E27EC < 0x13) {
            a1[3] = a1[3] - 1;
        }
        if (D_800E27EC < 0x18) {
            break;
        }
        return 1;
    case 2:
        buf[0] = a1[0];
        buf[1] = a1[1];
        buf[2] = a1[2];
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = a1[1] + a1[0] + D_800E27EC * 32;
        vec[3] = 0;
        s0v = func_80077CF4((D_800E27EC << 11) / 24) / 128;
        idx = D_800F336C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0x60, w);
        func_800CEE20(buf, vec, 0x1000, 0x1000, D_800F336A * (D_800E27EC / 6) + 0x60, t & 0xFFFF, 1, s0v, 0);
        break;
    default:
        return 0;
    }
    return 0;
}
