typedef struct { char b[4]; } Blk4;

extern int D_800E27EC;
extern unsigned char D_800C22C0[];
extern int func_80077DC4();
extern void func_800D27FC();

int func_800D5898(int a0, short *a1) {
    Blk4 v18;
    int r;

    v18 = *(Blk4 *)D_800C22C0;
    switch (a0) {
    case 1:
        r = func_80077DC4((D_800E27EC << 10) / a1[3]);
        {
            int t = a1[1] * r / 0x1000;
            int lim = a1[3];
            int cur = D_800E27EC;
            a1[0] = t;
            if (cur >= lim) {
                return 1;
            }
        }
        break;
    case 2:
        r = func_80077DC4((D_800E27EC << 10) / a1[3]);
        {
            int d = r / 64 + 0x40;
            func_800D27FC(a1[0], a1[2], &v18, d, 1);
        }
        break;
    }
    return 0;
}
