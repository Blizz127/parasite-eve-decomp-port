extern unsigned char *D_800E2248;
extern unsigned char *D_800F34F4;
extern unsigned char *D_800F32A8;
extern unsigned char *D_800F3330;
extern int D_800F33B0;
extern unsigned char D_800C20EC[];
extern void func_80071A74(unsigned char *a0);
extern int func_800C6CE0(unsigned char *a0);
extern int func_800C2DA0(int a0);

int func_800C251C(unsigned char *a0, void **a1) {
    short i;
    unsigned char *s;
    int acc;
    void **q;
    unsigned char *ctx;
    short t;
    int f78;
    int st;

    s = a0;
    q = a1;
    acc = 0;
    f78 = *(int *)(s + 0x78);
    ctx = *(unsigned char **)(s + 8);
    D_800E2248 = s + 0xC;
    D_800F34F4 = s + 0x80;
    D_800F32A8 = s;
    D_800F3330 = s + 0x200;
    D_800F33B0 = f78;
    s[3] = s[0x12];
    st = func_800C6CE0(s);
    i = 0;
    if (st == 3) {
        if (s[1] != 0x24) {
            unsigned char *r =
                *(unsigned char **)(*(unsigned char **)(*(unsigned char **)(s + 8)) + 0x18);
            if (r[0] == 1) {
                r[0] = 2;
                __asm__ __volatile__("" : "=r"(i));
                i = 0;
            }
        }
    }
    do {
        unsigned char *e = (unsigned char *)(i * 6 + (unsigned int)D_800F34F4);
        if (*(signed char *)(e + 1) == 1) {
            void (*f)(unsigned char *, unsigned char *, unsigned char *);
            f = (void (*)(unsigned char *, unsigned char *, unsigned char *))q[*e];
            if (f != (void (*)(unsigned char *, unsigned char *, unsigned char *))-1) {
                f(s, e, D_800F3330 + *(short *)(e + 4));
            } else {
                func_80071A74(D_800C20EC);
            }
            {
                unsigned char *e2 = (unsigned char *)(i * 6 + (unsigned int)D_800F34F4);
                *(unsigned short *)(e2 + 2) = *(unsigned short *)(e2 + 2) + 1;
            }
        }
        {
            unsigned char *e3 = (unsigned char *)(i * 6 + (unsigned int)D_800F34F4);
            if (*(signed char *)(e3 + 1) == 2) {
                acc |= func_800C2DA0((unsigned short)i);
                t = i + 1;
            } else {
                t = i + 1;
            }
        }
        i = t;
        __asm__ __volatile__("" : "=r"(i) : "0"(i));
    } while (t < 0x40);
    if (func_800C6CE0(s) == 3) {
        unsigned char *p = *(unsigned char **)ctx;
        unsigned int w = *(unsigned int *)p;
        if (*(unsigned char *)(p + ((w >> 17) & 0x70) + 0x1C) == 0) {
            if (w & 0x180E) {
                acc = -1;
            }
        }
    }
    return acc;
}
