extern int D_800E27EC;
extern unsigned char *D_800E2368;
extern char D_800E141C;
extern char D_800E1444;
extern char D_800E146C;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern void func_800CF3AC();
extern int func_80077AA4();
extern void func_800CEE20();

int func_800D4928(int a0, short *a1)
{
    short buf[4];
    short vec[4];
    int blk[2];
    char *src;
    int s0v;
    int w;
    int idx;
    int t;
    int x;

    switch (a0) {
    case 1:
        a1[0] = a1[0] + a1[3];
        a1[1] = a1[1] + a1[4];
        a1[2] = a1[2] + a1[5];
        a1[3] = a1[3] * 6 / 7;
        a1[5] = a1[5] * 6 / 7;
        if (a1[1] > 0) {
            x = a1[4];
            a1[4] = -x;
        }
        a1[4] = a1[4] + 3;
        if (D_800E27EC < a1[6]) {
            break;
        }
        return 1;
    case 2:
        x = *(short *)(D_800E2368 + 0x1E);
        switch (x) {
        case 0:
            src = &D_800E141C;
            break;
        case 1:
            src = &D_800E1444;
            break;
        default:
            src = &D_800E146C;
            break;
        }
        func_800CF3AC(src, blk, D_800E27EC * 48 / a1[6]);
        buf[0] = a1[0];
        buf[1] = a1[1];
        buf[2] = a1[2];
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = a1[7] + D_800E27EC * 128;
        vec[3] = 0;
        s0v = (D_800E27EC << 11) / a1[6] + 0x400;
        idx = D_800F336C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0x20, w) & 0xFFFF;
        func_800CEE20(buf, vec, s0v, s0v, D_800F336A * (D_800E27EC % 7) + 0xE8, t, 1, 0x80, blk);
        break;
    default:
        return 0;
    }
    return 0;
}
