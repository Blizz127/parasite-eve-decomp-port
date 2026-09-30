extern int D_800E27EC;
extern char D_800E1988;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80077AA4(int a0, int a1);
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800CEE20(short *a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int *a8);

int func_800D7E78(int a0, short *a1)
{
    short buf[4];
    int buf2[2];
    int s0v;
    int w;
    int idx;
    int t;

    switch (a0) {
    case 1:
        a1[1] = a1[1] - 1;
        if (D_800E27EC < 6) {
            break;
        }
        return 1;
    case 2:
        buf[0] = a1[0];
        buf[1] = a1[1];
        buf[2] = a1[2];
        func_800CF3AC(&D_800E1988, buf2, D_800E27EC);
        s0v = 0x1000 - (D_800E27EC << 12) / 6;
        idx = D_800F336C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0x70, w);
        func_800CEE20(buf, 0, s0v, s0v, 0x8A, t & 0xFFFF, 1, 0x80, buf2);
        break;
    default:
        return 0;
    }
    return 0;
}
