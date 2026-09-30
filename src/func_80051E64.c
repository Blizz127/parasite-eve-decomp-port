typedef struct {
    unsigned int a : 10;
    unsigned int b : 10;
    unsigned int c : 8;
    unsigned int pad0 : 4;
    unsigned int f0 : 1;
    unsigned int f1 : 1;
    unsigned int f2 : 1;
    unsigned int f3 : 1;
    unsigned int f4 : 1;
    unsigned int d : 4;
    unsigned int f9 : 1;
    unsigned int f10 : 1;
    unsigned int f11 : 1;
    unsigned int f12 : 1;
    unsigned int f13 : 1;
    unsigned int f14 : 1;
    unsigned int f15 : 1;
    unsigned int f16 : 1;
    unsigned int f17 : 1;
    unsigned int pad1 : 14;
} Stat;

extern signed char D_800C0E22[];
extern void func_80052E30(int);
extern unsigned char *func_8005332C(int);
extern void func_80051CC4(void);

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

void func_80051E64(Stat *p)
{
    unsigned char *q;
    int t;
    int v;
    int i;

    *((unsigned int *)p + 1) = 0;
    func_80052E30(0);
    q = func_8005332C(D_800C0E22[0]);
    if (q != 0) {
        { int u = q[7] + *(short *)(q + 0xE);
        p->a = u >= 1000 ? 999 : u; }
        { int u = q[8] + *(short *)(q + 0x10);
        p->b = u >= 1000 ? 999 : u; }
        t = q[9] + *(short *)(q + 0x12);
        if (t >= 1000) {
            t = 999;
        }
        if (t < 85) {
            t = isqrt(t * 3000) / 10;
        } else {
            t = (t * 249 / 208 + 402) / 10;
        }
        p->c = t;
        for (i = 0; i < q[0x14]; i++) {
            v = (q + i)[0x15] & 0x1F;
            switch (v) {
            case 1: p->f0 = 1; break;
            case 2: p->f1 = 1; break;
            case 3: p->f2 = 1; break;
            case 4: p->f3 = 1; break;
            case 5: p->f4 = 1; break;
            case 6: p->f14 = 1; break;
            case 7: p->f15 = 1; break;
            case 8:
            case 9:
            case 10:
                p->d = 1 << (v - 8);
                break;
            case 11: p->f11 = 1; break;
            case 12: p->f13 = 1; break;
            case 13: p->f16 = 1; break;
            case 14: p->f9 = 1; break;
            case 15: p->f17 = 1; break;
            case 16: p->f10 = 1; break;
            case 17: p->f12 = 1; break;
            }
        }
    } else {
        p->a = 0;
        p->b = 0;
        p->c = 0;
    }
    func_80051CC4();
}
