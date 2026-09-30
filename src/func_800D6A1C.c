typedef struct {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
} B4;

extern B4 D_800C22CC;
extern int D_800E27EC;
extern unsigned char *D_800F32D0;
extern short D_800F3374;
extern int func_80071A54(void);
extern int func_80077CF4(int a0);
extern int func_80077DC4(int a0);
extern void func_800D004C();
extern void func_800D0728();

int func_800D6A1C(int a0, short *a1)
{
    short vec[4];
    B4 blk;
    int s1v;
    int s2v;
    unsigned char *q;
    short *f;
    int t;

    blk = D_800C22CC;
    switch (a0) {
    case 0:
        t = func_80071A54();
        q = D_800F32D0;
        *(int *)(a1 + 8) = t;
        a1[0] = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x268);
        a1[1] = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x26A);
        a1[2] = *(unsigned short *)(*(unsigned char **)(q + 8) + 0x26C);
        a1[4] = a1[0];
        a1[5] = a1[1];
        a1[6] = a1[2];
        a1[5] = a1[5] - 0x1B8;
        return 0;
    case 1:
        if (D_800E27EC < 0x20) {
            break;
        }
        return 1;
    case 2:
        f = &D_800F3374;
        *f = 0x64;
        s1v = func_80077DC4(D_800E27EC << 5) / 2 + 0x800;
        s2v = func_80077DC4(D_800E27EC << 5) / 32;
        func_800D004C(a1, 0x2BC, 0x2BC, 0xC, 0, s1v, s1v, &blk, 0, s2v, 1);
        *f = 0;
        if (D_800E27EC < 0x11) {
            s2v = 0x80 - (D_800E27EC << 3);
            vec[0] = 0x400;
            vec[1] = 0;
            vec[2] = D_800E27EC << 7;
            vec[3] = 1;
            s1v = func_80077CF4(D_800E27EC << 6) * 2;
            func_800D0728(a1, 0x320, 0x3E8, 0x10, vec, s1v, s1v, 0, &blk, s2v, 1);
        }
        return 0;
    }
    return 0;
}
