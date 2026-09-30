typedef struct Font {
    unsigned char f1C;
    unsigned char f1D;
    unsigned char f1E;
    unsigned char cur;
    unsigned char f20;
    unsigned char pad5[3];
    int seed;
    unsigned char *data;
} Font;

typedef struct Rd {
    int base;
    unsigned char pad4[0x90];
    unsigned char *buf;
} Rd;

extern Font D_80091A1C;
extern Rd D_800B0DD8;
extern unsigned short D_80093176[];
extern unsigned char D_8009ECD8[];
extern unsigned char D_8009EE24[];
extern int func_8006E6A8(int, unsigned char *, int);
extern int func_8006E7E8(void);
extern int func_80071A54(void);
extern void func_80071A64(int);
extern int func_800389DC(int);

static inline unsigned char cur_attr(void)
{
    unsigned char *d = D_80091A1C.data;

    return (d + (d + D_80091A1C.cur)[29])[4];
}

static inline int find_pos(unsigned char cls)
{
    unsigned char *p;
    unsigned char *q;
    int i;
    int r;
    unsigned char sel;

    sel = 0;
    p = D_80091A1C.data + 1;
    for (i = 0; i < p[2]; i++) {
        if ((p + i)[3] == cls) {
            sel = i;
            i = p[2];
        }
    }
    q = p + 27;
    for (i = 0; i < q[0]; i++) {
        if ((q + i)[1] == sel) {
            goto found;
        }
    }
    r = 255;
    goto done;
found:
    r = i;
done:
    return r;
}

unsigned char func_80038D74(void)
{
    int v;
    unsigned char i;
    unsigned char j;
    unsigned char n;
    short cnt;
    short lim;
    int k;
    unsigned char ok;

retry:
    while (func_8006E6A8(D_800B0DD8.base + D_80093176[0], D_800B0DD8.buf,
                         D_80093176[1] - D_80093176[0]) == -1) {
    }
    while ((v = func_8006E7E8()) != 0) {
        if (v == -1) {
            goto retry;
        }
    }
    D_8009ECD8[0] = D_800B0DD8.buf[0];
    if (D_80091A1C.f20 == 0 && D_80091A1C.seed == 0) {
        while ((D_80091A1C.seed = func_80071A54() % 32767) == 0) {
        }
    }
    do {
        func_80071A64(D_80091A1C.seed);
    } while (0);
    cnt = 0;
    n = 0;
    i = 0;
    for (; i < 69; i++) {
        k = i + 2;
        if (k % 10 == 0) {
            D_8009EE24[i] = n;
            n++;
            continue;
        }
        if (k < 10) {
            lim = 104;
        } else if (k < 20) {
            lim = 109;
        } else {
            lim = 113;
        }
        if (D_8009ECD8[0] - 1 < lim) {
            lim = D_8009ECD8[0] - 1;
        }
        D_8009EE24[i] = func_80071A54() % (lim - 7) + 7;
        if (cnt < D_8009ECD8[0] - 9) {
            do {
                ok = 1;
                for (j = 0; j < i; j++) {
                    if (D_8009EE24[j] == D_8009EE24[i]) {
                        D_8009EE24[i] = func_80071A54() % (lim - 7) + 7;
                        ok = 0;
                    }
                }
            } while (!ok);
        }
        cnt++;
    }
    func_800389DC(D_8009EE24[0]);
    D_80091A1C.f1C = 1;
    D_80091A1C.f1D = 2;
    D_80091A1C.f1E = D_8009EE24[0];
    D_80091A1C.data = D_8009ECD8;
    D_80091A1C.cur = find_pos(2);
    return cur_attr();
}
