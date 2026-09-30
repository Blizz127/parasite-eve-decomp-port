extern short *D_8009D048;
extern int D_8009D050;
extern int D_8009D040;
extern signed char D_800C0E22[];
extern short D_800A1D9C[];
extern unsigned char D_8009DE64[];
extern unsigned char D_800BEEAC[];
extern unsigned char *func_8005DB44(int);
extern int func_80054E4C(int);
extern int func_80054CF8(void);
extern int *func_80051098(void);
extern void func_800512AC(int, int);

int func_8005968C(int sel)
{
    int unused[4];
    unsigned char *p;
    unsigned char *res;
    int v;
    register int w asm("$5");
    register int m asm("$3");
    int i;
    int n;
    int nn;
    int t;
    register int m1 asm("$19");
    int m2;
    int old;
    int idx;
    int nx;
    int *q;
    int r;
    register int c3 asm("$3");
    register int ca asm("$4");

    m1 = 0;
    if (D_800C0E22[0] >= 0 && D_800C0E22[0] < D_8009D050) {
        v = D_8009D048[D_800C0E22[0]];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + ((int)D_800C0E22 - 0x1F76));
            goto e1;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto e1;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            m = w << 5;
            res = (unsigned char *)(m + (int)D_8009DE64);
            goto e1;
        }
        res = 0;
    e1:
        p = res;
    } else {
        p = 0;
    }
    if (p != 0) {
        n = p[0x14];

        i = 0;

        if (n > 0) {
            nn = n;
        al:
            t = (p + i)[0x15] & 0x1F;
            m1 = t - 8;
            if ((unsigned int)m1 >= 3) {
                i++;
                if (i < nn) {
                    goto al;
                }
            }
            if (i < p[0x14]) {
                m1 = 1 << m1;
            } else {
                m1 = 0;
            }
        } else {
            m1 = 0;
        }
    }
    idx = 0;
    if (sel >= 0 && sel < D_8009D040) {
        idx = D_800A1D9C[sel];
    }
    if (idx >= 0 && idx < D_8009D050) {
        v = D_8009D048[idx];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + ((int)D_800BEEAC));
            goto e2;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto e2;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            m = w << 5;
            res = (unsigned char *)(m + (int)D_8009DE64);
            goto e2;
        }
        res = 0;
    e2:
        p = res;
    } else {
        p = 0;
    }
    n = p[0x14];

    i = 0;

    if (n > 0) {
        nn = n;
    bl:
        t = (p + i)[0x15] & 0x1F;
        m2 = t - 8;
        if ((unsigned int)m2 >= 3) {
        i++;
        if (i < nn) {
            goto bl;
        }
        }
        if (i < p[0x14]) {
        m2 = 1 << m2;
        } else {
        m2 = 0;
        }
    } else {
        m2 = 0;
    }
    old = D_800C0E22[0];
    if (old >= 0 && old < D_8009D050) {
        v = D_8009D048[old];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + ((int)D_800C0E22 - 0x1F76));
            goto e3;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto e3;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            m = w << 5;
            res = (unsigned char *)(m + (int)D_8009DE64);
            goto e3;
        }
        res = 0;
    e3:
        p = res;
    } else {
        p = 0;
    }
    if (sel >= 0 && sel < D_8009D040) {
        nx = D_800A1D9C[sel];
        goto e4;
    }
    nx = 0;
e4:
    D_800C0E22[0] = nx;
    r = func_80054E4C(m1 - m2);
    func_80054CF8();
    if (r == 1) {
        q = func_80051098();
        ca = 3;
        __asm__ __volatile__("" : "=r"(ca) : "0"(ca));
        c3 = 3;
        q[0] = c3;
        q[1] = (int)p;
        func_800512AC(ca, 0);
    } else {
        D_800C0E22[0] = old;
    }
    return r;
}
