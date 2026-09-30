extern int D_800E27EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80077AA4(int a0, int a1);
extern int func_80077CF4(int a0);
extern void func_800CEE20(short *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8);

int func_800DE7A8(int a0, short *a1)
{
    short pos[4];
    short vec[4];
    int s0v;
    int s1v;
    int w;
    int idx;
    int t;
    int e;

    switch (a0) {
    case 1:
        a1[0] = a1[0] + a1[3];
        a1[1] = a1[1] + a1[4];
        a1[2] = a1[2] + a1[5];
        a1[4] = a1[4] + 1;
        a1[3] = a1[3] * 31 / 32;
        a1[5] = a1[5] * 31 / 32;
        if (D_800E27EC < 0x20) {
            break;
        }
        return 1;
    case 2:
        e = a1[7];
        s1v = e - e * D_800E27EC / 32;
        pos[0] = a1[0];
        pos[1] = a1[1];
        pos[2] = a1[2];
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = a1[6] + (D_800E27EC << 3);
        s0v = func_80077CF4(D_800E27EC << 5) + 0x2800;
        idx = D_800F336C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        func_800CEE20(pos, vec, s0v, s0v, D_800F336A * (D_800E27EC / 4) + 0xC0, func_80077AA4(0x40, w) & 0xFFFF, 1, s1v, 0);
        s0v = 0x1800;
        idx = D_800F336C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        func_800CEE20(pos, vec, s0v, s0v, D_800F336A * (D_800E27EC / 8) + 0x60, func_80077AA4(0, w) & 0xFFFF, 3, s1v, 0);
        break;
    default:
        return 0;
    }
    return 0;
}
