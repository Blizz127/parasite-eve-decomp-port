extern void func_80085174(void);
extern unsigned int func_80085084(const unsigned int *a0);
extern int func_80085EB4(int a0);
extern void func_800850F4(int arg0, int arg1);
extern volatile unsigned int D_8009D24C;
extern unsigned char D_800B2900[];

int func_800851A8(unsigned int *buf, int count) {
    register unsigned int *s0 asm("$16") = buf;
    register unsigned int *s1 asm("$17");
    register unsigned int s2 asm("$18");
    register int s3 asm("$19") = count;
    unsigned int size;

    func_80085174();
    if (func_80085084(s0) != 0) {
        goto bad;
    }
    s0 = s0 + 4;
    func_80085EB4((int)*s0);
    s0 = s0 + 1;
    asm volatile("" : "=r"(s0) : "0"(s0));
    size = *s0;
    s0 = s0 + 1;
    asm volatile("" : "=r"(s0) : "0"(s0));
    s2 = *s0;
    s0 = s0 + 1;
    asm volatile("" : "=r"(s0) : "0"(s0));
    {
        register unsigned int mode asm("$2");
        register unsigned char *base asm("$2");
        register unsigned int *dst asm("$3");
        register unsigned int left asm("$5");
        unsigned int n;
        unsigned int w;

        mode = *s0;
        s1 = s0 + 9;
        if (mode == 0) {
            mode = 256;
        }
        n = mode - s2;
        func_800850F4((int)s1 + (int)(n << 6), (int)size);
        dst = (unsigned int *)(s2 << 6);
        base = D_800B2900;
        left = n << 4;
        dst = (unsigned int *)((unsigned int)dst + (unsigned int)base);
        if (left != 0) {
            do {
                left = left - 1;
                w = *s1;
                s1 = s1 + 1;
                *dst = w;
                dst = dst + 1;
            } while (left != 0);
        }
    }
    if (s3 != 0) {
        func_80085174();
    }
    return 0;
bad:
    D_8009D24C = 0xFFFFFFFFu;
    return -1;
}
