extern int D_8009D108;
extern unsigned int *D_8009D100;
extern unsigned int D_8009D104;
extern unsigned int *D_8009D11C;
extern void func_800527C0(int);
extern void func_80075B84(unsigned int *, short *);

void func_8005E9C8(int a0, int a1, int a2, int a3)
{
    short buf[4];
    short *bp;
    unsigned int *p;
    unsigned int *q;

    if (D_8009D108 != 0) {
        a1 += 0xE0;
    }
    bp = buf;
    p = 0;
    buf[0] = a0;
    buf[1] = a1;
    buf[2] = a2;
    buf[3] = a3;
    if (D_8009D100 + 3 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 3;
        p = D_8009D100 - 3;
    } else {
        func_800527C0(1);
    }
    if (p != 0) {
        func_80075B84(p, bp);
    }
    q = D_8009D11C;
    *p = (*p & 0xFF000000) | (*q & 0xFFFFFF);
    *q = (*q & 0xFF000000) | ((unsigned int)p & 0xFFFFFF);
}
