extern int D_800E27EC;
extern int D_8009D254;
extern char D_800E1FA4;
extern char D_800E1FCC;
extern short D_800F3374;
extern int func_80071A54(void);
extern void func_800CE870(int a0, int a1, short *a2);
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800D004C(short *a0, int a1, int a2, int a3, short *a4, int a5, int a6, int *a7, int a8, int a9, int a10);
extern void func_800D0728(short *a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int *a8, int a9, int a10);

int func_800DD19C(int a0, short *a1)
{
    short buf[4];
    short vec[4];
    int blk[2];
    int s0v;
    int r;
    int p;

    switch (a0) {
    case 0:
        r = func_80071A54();
        p = D_8009D254;
        *(int *)(a1 + 4) = r;
        func_800CE870(p, 1, a1);
        a1[1] = a1[1] - 0x1FE;
        return 0;
    case 1:
        if (D_800E27EC < 0x40) {
            break;
        }
        return 1;
    case 2:
        D_800F3374 = 0x3C;
        buf[0] = a1[0];
        buf[1] = a1[1];
        buf[2] = a1[2];
        s0v = D_800E27EC * 32 + 0x1000;
        func_800CF3AC(&D_800E1FA4, blk, D_800E27EC);
        func_800D004C(buf, 0x1F4, 0x1F4, 0x10, 0, s0v, s0v, blk, 0, 0x80, 1);
        func_800D0728(buf, 0x19A, 0x1F4, 0x14, 0, 0x1000, 0x1000, 0, blk, 0x80, 3);
        func_800CF3AC(&D_800E1FCC, blk, D_800E27EC);
        vec[0] = 0;
        vec[1] = 0;
        vec[3] = 0;
        vec[2] = D_800E27EC * 16;
        func_800D004C(buf, 0x190, 0x64, 0x18, vec, s0v, s0v, blk, 0, 0x80, 1);
        return 0;
    }
    return 0;
}
