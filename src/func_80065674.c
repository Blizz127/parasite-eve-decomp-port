extern unsigned int D_8009D1A0;
extern unsigned int D_800B0CD8;
extern unsigned char *D_800B1624;
extern unsigned char D_800BCFFD;

typedef struct {
    union { unsigned int w; unsigned char b; } hdr;
    union { int w; unsigned char b; } pos;
    short speed;
    unsigned short tick;
    int off;
} Rec;

int func_80065674(void)
{
    unsigned char *h;
    Rec *r;
    Rec *recs;
    unsigned char *m;
    unsigned char *l;
    unsigned char *q;
    unsigned int i;
    unsigned int n;
    unsigned int j;
    unsigned int cnt;
    int t1;
    int a1;
    int a3;
    int hi;
    int v;
    unsigned int c2;

    if (D_8009D1A0 & 0x104) {
        goto out;
    }
    if ((D_800B0CD8 & 0xC00000) == 0x800000) {
        goto out;
    }
    h = D_800B1624;
    recs = (Rec *)(h + *(int *)(h + 0x10));
    m = h + *(int *)(h + 0x14);
    n = *(unsigned short *)(h + 4);
    for (i = 0; i < n; i++) {
        r = &recs[i];
        t1 = r->pos.w >> 8;
        if (!(r->hdr.b & 2) || !(r->hdr.b & 0x14)) {
            continue;
        }
        if (r->pos.b != D_800BCFFD) {
            continue;
        }
        j = 0;
        l = (unsigned char *)r + *(volatile int *)&r->off;
        cnt = *(volatile unsigned int *)&r->hdr.w >> 8;
        for (; j < cnt; j++) {
            m[l[j * 2] * 56] &= ~2;
        }
        q = (unsigned char *)((t1 >> 8) * 2 + (int)l);
        m[q[0] * 56] |= 2;
        if ((signed char)q[1] < 0) {
            q[1] = 0;
            r->tick = 0;
            r->hdr.b &= ~4;
            return 0;
        }
        r->tick++;
        if (r->tick < (signed char)q[1]) {
            continue;
        }
        a1 = r->pos.b;
        a1 |= (t1 + r->speed) << 8;
        r->tick = 0;
        r->pos.w = a1;
        a3 = a1 >> 8;
        hi = a1 >> 16;
        c2 = r->hdr.w >> 8;
        if (hi >= (int)c2) {
            if (r->hdr.b & 0x20) {
                v = (a1 & 0xFF) | ((a3 % (int)(c2 << 8)) << 8);
            } else {
                v = a1 & 0xFF;
            }
            r->pos.w = v;
        } else if (hi < 0) {
            if (r->hdr.b & 0x20) {
                r->pos.w = (a1 & 0xFF) | (((c2 << 8) - (-a3 % (int)(c2 << 8))) << 8);
            } else {
                r->pos.w = (a1 & 0xFF) | ((c2 - 1) << 16);
            }
        } else {
            continue;
        }
        r->hdr.b &= ~4;
    }
out:
    return 0;
}
