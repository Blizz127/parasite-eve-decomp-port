extern int D_8009D050;
extern short *D_8009D048;
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern short D_800C0E48[];
extern signed char D_800C0E22[];
extern unsigned char *func_8005DB44(int);
extern int func_80059F08(int);
extern int func_80054E4C(int);

static inline unsigned char *rec(int a0)
{
    int v;
    int w;
    unsigned char *res;

    if (a0 >= 0 && a0 < D_8009D050) {
        v = D_8009D048[a0];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto done;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto done;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            res = (unsigned char *)((w << 5) + (int)D_8009DE64);
            goto done;
        }
        res = 0;
    done:
        return res;
    }
    return 0;
}

int func_80054F58(int side, int k)
{
    int s_me;
    int s_other;
    unsigned char *p;
    unsigned char *q;
    register int r asm("$19");
    int m;
    int i;
    register int a asm("$4");
    int b;
    int d;
    int x;
    int y;

    m = 0;
    s_me = func_80059F08(side);
    p = rec(s_me);
    s_other = func_80059F08(side == 0);
    q = rec(s_other);
    r = 1;
    if (k >= 0) {
        a = (p + k)[0x15];
        for (i = 0; i < q[0x14]; i++) {
            if ((q + i)[0x15] == a) {
                break;
            }
        }
        if (i < q[0x14]) {
            return 4;
        }
        for (i = 0; i < q[0x14]; i++) {
            if ((q + i)[0x15] == 0) {
                break;
            }
        }
        if (i >= q[0x14]) {
            a = (p + k)[0x15] & 0xE0;
            if (a == 0) {
                return 3;
            }
            asm volatile("");
            i = 0;
            if (i < q[0x14]) {
            register int c asm("$5");
            c = a;
            for (; i < q[0x14]; i++) {
                if (((q + i)[0x15] & 0xE0) == c) {
                    break;
                }
            }
            }
            if (i >= q[0x14]) {
                return 3;
            }
        }
    }
    func_80059F08(side);
    if (D_8009D048 == D_800C0E48 && s_me == D_800C0E22[0]) {
        for (i = 0; i < p[0x14]; i++) {
            x = ((p + i)[0x15] & 0x1F) - 8;
            if ((unsigned int)x < 3) {
                break;
            }
        }
        if (i < p[0x14]) {
            d = i != k;
            r = func_80054E4C(1 << x);
            if (r == 0) {
                asm("" : "=r"(r) : "0"(r));
                if (d) {
                    goto five;
                }
                if (k >= 0) {
                    return r;
                }
            five:
                r = 5;
            }
        }
        return r;
    }
    func_80059F08(side == 0);
    if (D_8009D048 != D_800C0E48) {
        return 1;
    }
    if (s_other != D_800C0E22[0]) {
        goto ret1;
    }
    if (k < 0) {
        goto ret1;
    }
    x = ((p + k)[0x15] & 0x1F) - 8;
    if ((unsigned int)x >= 3) {
        return 1;
    }
    for (i = 0; i < q[0x14]; i++) {
        y = ((q + i)[0x15] & 0x1F) - 8;
        if ((unsigned int)y < 3) {
            break;
        }
    }
    if (i < q[0x14]) {
        m = (1 << y) - (1 << x);
    }
    r = func_80054E4C(m);
    goto out;
ret1:
    return 1;
out:
    return r;
}
