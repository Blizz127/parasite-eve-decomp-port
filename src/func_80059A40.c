extern signed char D_800C0E22[];
extern unsigned char D_800C0E0C;
extern unsigned char D_800C0E48[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char D_8009D05C[];
extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char *D_8009D058;
extern int D_8009D064;
extern unsigned char *func_8005DB44(int);
extern int func_80051E58(void);
extern int func_80052F70(void);

int func_80059A40(int *out)
{
    int unused[2];
    unsigned char *p;
    unsigned char *res;
    unsigned char *q2;
    short *q;
    int idx;
    int v;
    int w;
    int m;
    int i;
    int n;
    int nn;
    int t;
    int cap;
    int c2;
    int used;
    int r;

    m = 0;
    idx = D_800C0E22[0];
    if (idx >= 0 && idx < D_8009D050) {
        v = D_8009D048[idx];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + ((int)D_800C0E22 - 0x1F76));
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
        p = res;
    } else {
        p = 0;
    }
    if (p != 0) {
        n = p[0x14];
        if (n > 0) {
            i = 0;
            nn = n;
        loop:
            t = (p + i)[0x15] & 0x1F;
            m = t - 8;
            if ((unsigned int)m >= 3) {
                i++;
                if (i < nn) {
                    goto loop;
                }
            }
            if (i < p[0x14]) {
                m = 1 << m;
            } else {
                m = 0;
            }
        } else {
            m = 0;
        }
    }
    c2 = (D_800C0E0C + func_80051E58() < 0x33) ? (D_800C0E0C + func_80051E58()) : 0x32;
    used = 0;
    D_8009D048 = (short *)D_800C0E48;
    cap = c2;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    q = D_8009D048;
    while (q < D_8009D048 + D_8009D050) {
        used += (*q != 0);
        q++;
    }
    r = (cap - used) >= m;
    if (out != 0) {
        *out = m;
    }
    return r;
}
