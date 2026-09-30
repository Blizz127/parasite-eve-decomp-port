extern unsigned int *D_8009D300;
extern int D_8009CE00;
extern unsigned short D_8009D2A4;
extern void func_80067CBC(void);
extern void func_8004C34C(int a0);

int func_80019540(int **a0) {
    unsigned short *d;
    unsigned short f;
    unsigned short v;

    d = (unsigned short *)D_8009D300;
    f = d[4];
    if ((f & 0x20) == 0) {
        d[4] = f | 0x20;
        func_80067CBC();
        D_8009CE00 -= 0x10;
        D_8009D300[4] = 1;
        return 0;
    }
    v = D_8009D2A4;
    if ((unsigned int)(v - 3) < 0x180) {
        *a0[1] = (short)v - 3;
    } else if ((short)v == -1) {
        *a0[1] = (short)v;
    } else {
        func_8004C34C(**a0);
        D_8009CE00 -= 0x10;
        D_8009D300[4] = 1;
        return 0;
    }
    ((unsigned short *)D_8009D300)[4] &= 0xFFDF;
    return 1;
}
