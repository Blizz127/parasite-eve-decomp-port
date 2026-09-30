extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char D_800A1E64[];

typedef struct {
    unsigned char pad0[6];
    unsigned char kind;
    unsigned char pad7[2];
    unsigned char b9;
    unsigned short wA;
    unsigned char padC[6];
    short s12;
} Rec;

int func_80053B48(Rec *a0)
{
    int r;
    unsigned int c;
    int k;
    int tag;
    short *q;
    int i;
    int t;
    int f;
    Rec *e;
    int cap;
    int b;
    int sum;
    int m;

    c = a0->kind;
    r = 0;
    if (c < 0x10) {
        if (c != 0 && c < 8) {
            if ((int)(c - 4) > 0) {
                k = c - 5;
            } else {
                k = 0;
            }
        } else if (c < 0x13) {
            k = -1;
        } else {
            k = c - 0x13;
        }
    } else {
        k = c - 0x10;
    }

    if ((unsigned int)k < 3) {
        tag = k + 0x200;
        q = D_8009D048;
        while (q < D_8009D048 + D_8009D050 && *q != tag) {
            q++;
        }
        if (q < D_8009D048 + D_8009D050) {
            i = q - D_8009D048;
            goto d1;
        }
        i = -1;
    d1:
        if (i < 0) {
            q = D_8009D048;
            while (q < D_8009D048 + D_8009D050 && *q != 0) {
                q++;
            }
            if (q < D_8009D048 + D_8009D050) {
                t = q - D_8009D048;
                goto d2;
            }
            t = -1;
        d2:
            f = t;
            if (f >= 0) {
                short *bp = D_8009D048;
                bp[f] = k + 0x200;
            } else {
                r = 1;
            }
        }
        e = (Rec *)(D_800A1E64 + (k << 5));
        sum = e->wA + a0->wA;
        e->wA = sum;
        cap = e->b9 + e->s12;
        m = sum & 0xFFFF;
        if (cap < 1000) {
            if (cap >= m) {
                return r;
            }
        } else {
            if (m < 1000) {
                return r;
            }
        }
        b = e->b9;
        e->wA = (b + e->s12 < 1000) ? (e->s12 + b) : 999;
    } else {
        r = 1;
    }
    return r;
}
