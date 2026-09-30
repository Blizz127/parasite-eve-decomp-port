extern unsigned int func_80085084(const unsigned int *a0);
extern int func_80085EB4(int a0);
extern void func_800850F4(int arg0, int arg1);
extern void func_8008A02C(unsigned int *a0, unsigned int a1, int a2);
extern unsigned char D_800B4900[];

int func_800875FC(unsigned int bank, unsigned int *buf) {
    register unsigned int *s0 asm("$16");
    register unsigned int s5 asm("$21");
    register unsigned int *s1 asm("$17");
    register unsigned int s2 asm("$18");
    register unsigned int s3 asm("$19");
    register unsigned int s4 asm("$20");
    register unsigned int spu asm("$3");
    unsigned int n;

    s5 = bank;
    s0 = buf;
    if ((s5 & ~1u) != 0) {
        return 1;
    }
    if (func_80085084(s0) != 0) {
        return -1;
    }
    s0 = s0 + 5;
    asm volatile("" : "=r"(s0) : "0"(s0));
    s2 = *s0;
    s0 = s0 + 1;
    asm volatile("" : "=r"(s0) : "0"(s0));
    n = *s0;
    s0 = s0 + 1;
    asm volatile("" : "=r"(s0) : "0"(s0));
    s3 = *s0;
    if (s3 == 0) {
        s3 = 256;
    }
    s3 = s3 - n;
    s1 = s0 + 9;
    {
        register unsigned int sh asm("$2");
        sh = s3 << 6;
        s0 = (unsigned int *)((unsigned int)s1 + sh);
        spu = 0x68000u;
        sh = s5 << 13;
        s4 = sh + spu;
    }
    func_80085EB4((int)s4);
    func_800850F4((int)s0, (int)s2);
    func_8008A02C(s1, s4, (int)s3);
    {
        register unsigned char *base asm("$2");
        register unsigned int *dst asm("$4");
        unsigned int w;

        s4 = s5 << 10;
        base = D_800B4900;
        dst = (unsigned int *)(s4 + (unsigned int)base);
        s2 = s3 << 4;
        do {
            s2 = s2 - 1;
            w = *s1;
            s1 = s1 + 1;
            *dst = w;
            dst = dst + 1;
        } while (s2 != 0);
        asm volatile("" : "=r"(s2) : "0"(s2));
    }
    return 0;
}
