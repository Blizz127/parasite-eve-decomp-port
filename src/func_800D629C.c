typedef struct {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
} B4;

extern B4 D_800C22C8;
extern int D_800E27EC;
extern unsigned short D_800E21D4;
extern unsigned short D_800E21D8;
extern unsigned short D_800E21DA;
extern unsigned short D_800E21DC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80071A54();
extern int func_80077DC4();
extern int func_80077CF4();
extern int func_80077AA4();
extern void func_800CEE20();

int func_800D629C(int a0, short *a1)
{
    B4 blk;
    int s1v;
    int s2v;
    int r;
    int k;
    int w;
    int idx;
    int t;

    blk = D_800C22C8;
    switch (a0) {
    case 1:
        a1[1] = a1[5];
        a1[0] = D_800E21D8 + D_800E21D4 * func_80077DC4(a1[4]) / 4096;
        a1[2] = D_800E21DC + D_800E21D4 * func_80077CF4(a1[4]) / 4096;
        a1[5] = a1[5] - 7;
        a1[4] = a1[4] + 2;
        if (D_800E27EC < 0x18) {
            break;
        }
        return 1;
    case 2:
        s1v = (D_800E27EC << 11) / 24 + 0x800;
        s2v = 0x80 - (D_800E27EC << 7) / 24;
        r = func_80071A54();
        a1[1] = D_800E21DA - (r & 0x1F);
        s1v = s1v * (((a1[6] + D_800E27EC / 2) & 7) * 128 + 0x1000) / 4096;
        idx = D_800F336C;
        w = D_800E1204[idx];
        if (idx == 4 && D_800F3428 != 0) {
            w += 4;
        }
        t = func_80077AA4(0x40, w);
        k = (a1[6] + D_800E27EC / 2) & 7;
        func_800CEE20(a1, 0, s1v * 2, s1v, D_800F336A * k + 0x80, t & 0xFFFF, 1, s2v, &blk);
        break;
    default:
        return 0;
    }
    return 0;
}
