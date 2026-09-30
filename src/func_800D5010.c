extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80077AA4(int a0, int a1);
extern int func_80077CF4(int a0);
extern void func_800CEE20(short *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8);

int func_800D5010(int a0, short *a1)
{
    short pos[4];
    short vec[4];
    int s1v;
    int s2v;
    int q;
    int w;
    int idx;
    int t;

    switch (a0) {
    case 1:
        a1[0] = a1[0] + a1[3];
        a1[1] = a1[1] + a1[4];
        a1[2] = a1[2] + a1[5];
        a1[3] = a1[3] * 31 / 32;
        a1[5] = a1[5] * 31 / 32;
        if (a1[1] > 0) {
            a1[4] *= -1;
        }
        a1[4] = a1[4] + 3;
        if (D_800E27EC < a1[6]) {
            break;
        }
        return 1;
    case 2:
        q = (D_800E27EC << 7) / a1[6];
        pos[0] = a1[0];
        pos[1] = a1[1];
        pos[2] = a1[2];
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = a1[7] + (D_800E27EC << 3);
        s2v = 0x80 - q;
        s1v = func_80077CF4((D_800E27EC << 10) / a1[6]) + 0x1000;
        idx = D_800F336C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0x40, w);
        func_800CEE20(pos, vec, s1v, s1v, D_800F336A * ((D_800E27EC << 3) / a1[6]) + 0x80, t & 0xFFFF, 1, s2v, 0);
        break;
    default:
        return 0;
    }
    return 0;
}
