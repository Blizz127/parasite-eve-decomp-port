extern int *D_8009D154;
extern int *D_8009D100;
extern int D_8009D104;
extern int D_8009D108;
extern int *D_8009D11C;
extern void func_800527C0(int a0);
extern void func_80075B84(int *a0, short *a1);

void func_8006374C(int *n0)
{
    int acc[2];
    short prim[4];
    register int *n asm("$7");
    register int *nx asm("$8");
    register int *e asm("$6");
    register int *p asm("$5");
    register int i asm("$3");
    register int *dl asm("$16");
    register short *pp asm("$17");
    register int *ap asm("$9");
    register int c4 asm("$2");
    register int *b asm("$4");
    register int a asm("$5");
    register int bb asm("$6");
    register int yy asm("$4");
    register int x0 asm("$3");
    register int m1 asm("$3");
    register int m2 asm("$2");
    register int fl asm("$2");
    register int lo asm("$4");
    register int hi asm("$6");
    int *q;
    register int *t asm("$5");

    b = n0;
    ap = acc;
    acc[1] = 0;
    acc[0] = 0;
    n = b;
    if (b != 0) {
        do {
            *ap = *ap + n[6];
            nx = 0;
            acc[1] = acc[1] + n[7];
            e = D_8009D154;
            if (e != 0) {
                do {
                    i = 0;
                    p = e;
                    for (; i < 4; i++) {
                        if ((int *)p[2] == n) {
                            break;
                        }
                        p++;
                    }
                    c4 = (i < 4);
                    if (c4 != 0) {
                        nx = e;
                        goto found;
                    }
                    e = (int *)e[0];
                } while (e != 0);
            }
        found:
            n = nx;
        } while (n != 0);
    }
    m1 = b[15];
    m2 = b[13];
    a = m1 * m2;
    m1 = b[16];
    m2 = b[14];
    bb = m1 * m2;
    yy = acc[1];
    fl = D_8009D108;
    x0 = acc[0];
    dl = 0;
    if (fl != 0) {
        yy = yy + 0xE0;
    }
    prim[1] = yy;
    pp = prim;
    prim[0] = x0;
    prim[2] = a;
    prim[3] = bb;
    q = D_8009D100;
    if ((unsigned int)(q + 3) < (unsigned int)(D_8009D104 + 0x4000)) {
        D_8009D100 = q + 3;
        dl = q;
    } else {
        func_800527C0(1);
    }
    if (dl != 0) {
        func_80075B84(dl, pp);
    }
    lo = 0xFFFFFF;
    hi = 0xFF000000;
    t = D_8009D11C;
    *dl = (*dl & hi) | (*t & lo);
    lo = (int)dl & lo;
    *t = (*t & hi) | lo;
}
