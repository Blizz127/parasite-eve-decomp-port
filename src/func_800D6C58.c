typedef struct {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
} B4;

extern B4 D_800C22D0;
extern int D_800E27EC;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80077AA4(int a0, int a1);
extern void func_800CEE20(short *a0, short *a1, int a2, int a3, int a4, int a5, int a6, int a7, B4 *a8);

int func_800D6C58(int a0, short *a1)
{
    short buf[4];
    short vec[4];
    B4 blk;
    int s0v;
    int w;
    int idx;
    int t;
    int sv;
    int q;

    blk = D_800C22D0;
    switch (a0) {
    case 1:
        a1[0] = a1[0] + a1[3];
        a1[1] = a1[1] + a1[4];
        a1[2] = a1[2] + a1[5];
        a1[3] = a1[3] * 31 / 32;
        a1[5] = a1[5] * 31 / 32;
        if (a1[1] > 0) {
            q = a1[4];
            a1[4] = -q;
        }
        a1[4] = a1[4] + 3;
        if (D_800E27EC < a1[6]) {
            break;
        }
        return 1;
    case 2:
        buf[0] = a1[0];
        buf[1] = a1[1];
        buf[2] = a1[2];
        vec[0] = 0;
        vec[1] = 0;
        vec[2] = ((int)a1[7] * 256) + D_800E27EC * 128;
        sv = 0x80;
        idx = D_800F336C;
        w = D_800E1204[idx];
        s0v = (a1[7] & 0x7FF) + 0x400;
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0x80, w);
        func_800CEE20(buf, vec, s0v, s0v, 0xBC, t & 0xFFFF, 0xFF, sv, &blk);
        break;
    default:
        return 0;
    }
    return 0;
}
