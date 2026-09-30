typedef struct {
    short f0;
    short f2;
    short pad4;
    short f6;
    int pad8;
    unsigned int a : 10;
    unsigned int b : 10;
    unsigned int g : 2;
    unsigned int padC : 10;
    unsigned int lo4 : 4;
    unsigned int b45 : 2;
    unsigned int b67 : 2;
    unsigned int f8 : 1;
    unsigned int f9 : 1;
    unsigned int f10 : 1;
    unsigned int f11 : 1;
    unsigned int f12 : 1;
    unsigned int b13 : 2;
    unsigned int f15 : 1;
    unsigned int f16 : 1;
    unsigned int f17 : 1;
    unsigned int pad10 : 14;
} Stat;

extern signed char D_800C0E20[];
extern unsigned char D_800923D0[];
extern void func_80052E30(int);
extern unsigned char *func_8005332C(int);

static inline int isqrt(int value)
{
    int result;
    int shift;
    int trial;

    result = 0;
    shift = 0x1E;
    do {
        trial = ((result << 2) + 1) << shift;
        result <<= 1;
        if (value >= trial) {
            value -= trial;
            result |= 1;
        }
        shift -= 2;
    } while (shift >= 0);
    return result;
}

void func_80051980(int unused, Stat *r)
{
    unsigned char *q;
    int k;
    int v;
    int i;

    func_80052E30(0);
    q = func_8005332C(D_800C0E20[0]);
    r->f0 = *(short *)(q + 0xE) + q[7] < 1000 ? q[7] + *(short *)(q + 0xE) : 999;
    {
        int u = q[8] + *(short *)(q + 0x10);
        r->f2 = isqrt(u < 1000 ? u * 22500 : 999 * 22500);
    }
    r->f6 = q[6];
    r->a = *(unsigned short *)(q + 0xA);
    {
        int u = q[9] + *(short *)(q + 0x12);
        r->b = u >= 1000 ? 999 : u;
    }
    if (q[6] != 0 && q[6] < 8) {
        k = q[6] - 4;
        if (k <= 0) {
            k = 1;
        }
    } else {
        k = (q[6] >= 19) ? q[6] - 18 : 0;
    }
    *(int *)((unsigned char *)r + 0x10) = 0x11;
    r->g = k;
    for (i = 0; i < q[0x14]; i++) {
        v = (q + i)[0x15] & 0x1F;
        switch (v) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            r->lo4 = D_800923D0[v];
            break;
        case 6:
        case 7:
        case 8:
            r->b67 = v - 5;
            break;
        case 9: r->f8 = 1; break;
        case 10: r->f9 = 1; break;
        case 11: r->f10 = 1; break;
        case 12: r->f11 = 1; break;
        case 13: r->f12 = 1; break;
        case 14: r->f15 = 1; break;
        case 15: r->f16 = 1; break;
        case 16:
        case 17:
            r->b13 = v - 0xF;
            break;
        case 18: r->f17 = 1; break;
        case 19:
        case 20:
            r->b45 = v - 0x11;
            break;
        }
    }
    if (r->b13 == 1) {
        r->f0 = r->f0 >> 1;
    }
}
