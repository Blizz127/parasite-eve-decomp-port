extern signed char D_800C0E20[];
extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char D_8009DE64[];
extern unsigned char D_800A1E64[];
extern unsigned char D_800A1E44[];
extern unsigned char *func_8005DB44(int);

#define CAP(p) ((((p)[9] + *(short *)((p) + 0x12)) > 999) ? 999 : ((p)[9] + *(short *)((p) + 0x12)))
#define HP(p) (*(unsigned short *)((p) + 0xA))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

int func_800574A8(void)
{
    unsigned char *p;
    unsigned char *res;
    unsigned char *q;
    int idx;
    int v;
    int w;
    unsigned int t;
    int amt;

    idx = D_800C0E20[0];
    p = 0;
    if (idx >= 0 && idx < D_8009D050) {
        v = D_8009D048[idx];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + ((int)D_800C0E20 - 0x1F74));
            goto done;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            p = func_8005DB44(v - 1);
            goto out;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            res = (unsigned char *)((w << 5) + (int)D_8009DE64);
            goto done;
        }
        res = 0;
    done:
        p = res;
    }
out:
    t = p[6];
    if (t != 0 && t < 8) {
        q = D_800A1E64;
        if ((int)(t - 4) > 0) {
            q = D_800A1E64 + ((t - 5) << 5);
        }
    } else if (t >= 0x13) {
        q = D_800A1E64 + ((t - 0x13) << 5);
    } else {
        q = D_800A1E44;
    }
    amt = MIN(CAP(p) - HP(p), HP(q));
    HP(q) -= amt;
    HP(p) += amt;
    return amt;
}
