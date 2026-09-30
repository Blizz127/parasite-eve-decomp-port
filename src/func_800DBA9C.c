extern int D_800E27EC;
extern int D_8009D254;
extern int D_800E1D60;
extern char D_800E1D64;
extern char D_800E1D84;
extern short D_800F3374;
extern int func_80077CF4(int a0);
extern int func_80077DC4(int a0);
extern void func_800CE870(int a0, int a1, short *a2);
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800D004C(short *a0, int a1, int a2, int a3, short *a4, int a5, int a6, int *a7, int a8, int a9, int a10);
extern void func_800D0728(short *a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7, int *a8, int a9, int a10);

int func_800DBA9C(int a0, short *a1)
{
    short buf[4];
    int blk[2];
    int s0v;
    int t;
    int p;

    switch (a0) {
    case 0:
        p = D_8009D254;
        t = D_800E1D60;
        *(int *)(a1 + 4) = t;
        D_800E1D60 = t + 0x555;
        func_800CE870(p, 0, a1);
        a1[1] = a1[1] - 100;
        a1[0] = a1[0] + func_80077DC4(*(int *)(a1 + 4)) * 300 / 4096;
        a1[2] = a1[2] + func_80077CF4(*(int *)(a1 + 4)) * 300 / 4096;
        return 0;
    case 1:
        if (D_800E27EC < 8) {
            break;
        }
        return 1;
    case 2:
        D_800F3374 = 0x3C;
        buf[0] = a1[0];
        buf[1] = a1[1];
        buf[2] = a1[2];
        s0v = (D_800E27EC << 9) + 0x800;
        func_800CF3AC(&D_800E1D64, blk, D_800E27EC);
        func_800D004C(buf, 0x1F4, 0x1F4, 0x10, 0, s0v, s0v, blk, 0, 0x80, 1);
        func_800D0728(buf, 0x19A, 0x1F4, 0x14, 0, 0x1000, 0x1000, 0, blk, 0x80, 3);
        func_800CF3AC(&D_800E1D84, blk, D_800E27EC);
        func_800D004C(buf, 0x1F4, 0x64, 0x10, 0, s0v, s0v, blk, 0, 0x80, 1);
        return 0;
    }
    return 0;
}
