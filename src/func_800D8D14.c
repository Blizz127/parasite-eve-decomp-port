extern int D_800E27EC;
extern char D_800E1AA0;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80077AA4(int a0, int a1);
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800CEE20(short *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, int *a8);

int func_800D8D14(int a0, short *a1)
{
    short buf[4];
    short vec[4];
    int blk[2];
    int s0v;
    int w;
    int idx;
    int t;

    switch (a0) {
    case 1:
        a1[1] = a1[1] - 3;
        if (D_800E27EC < 0xC) {
            break;
        }
        return 1;
    case 2:
        buf[0] = a1[0];
        buf[1] = a1[1];
        buf[2] = a1[2];
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = D_800E27EC * 64;
        vec[3] = 0;
        func_800CF3AC(&D_800E1AA0, blk, D_800E27EC);
        s0v = 0x1000 - (D_800E27EC << 12) / 12;
        idx = D_800F336C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0x70, w);
        func_800CEE20(buf, vec, s0v, s0v, 0x8A, t & 0xFFFF, 1, 0x80, blk);
        break;
    default:
        return 0;
    }
    return 0;
}
