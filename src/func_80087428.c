extern unsigned int D_8009D270;
extern unsigned int D_8009D1EC;
extern unsigned int D_8009D204;
extern unsigned char D_800B4D00[];
extern unsigned int func_80085084(const unsigned int *a0);
extern int func_80085EB4(int a0);
extern void func_800850F4(int arg0, int arg1);
extern void func_8008A02C(unsigned int *a0, unsigned int a1, int a2);

int func_80087428(unsigned int bank, unsigned int *buf, unsigned int bytes)
{
    register unsigned int s6 asm("$22");
    register unsigned int *s0 asm("$16");
    register unsigned int s2 asm("$18");
    register unsigned int s5 asm("$21");
    register unsigned int s3 asm("$19");
    register unsigned int *s4 asm("$20");
    register unsigned int s1 asm("$17");
    unsigned int n;
    unsigned int w;

    s6 = bank;
    s0 = buf;
    s2 = bytes;
    if ((D_8009D270 & 2) != 0) {
        if (func_80085084(s0) != 0) {
            return -1;
        }
        s0 = s0 + 5;
        asm volatile("" : "=r"(s0) : "0"(s0));
        s5 = *s0;
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
        s0 = s0 + 9;
        asm volatile("" : "=r"(s0) : "0"(s0));
        s4 = s0;
        s1 = s3 << 6;
        s0 = (unsigned int *)((unsigned int)s0 + s1);
        {
            register unsigned int v asm("$2");

            v = s2 - 0x40;
            asm volatile("" : "=r"(v) : "0"(v));
            s2 = v - s1;
            asm volatile("" : "=r"(s2) : "0"(s2));
        }
        s1 = s2;
        if (s1 >= s5) {
            s1 = s5;
        }
        {
            register unsigned int base asm("$3");
            register unsigned int mul asm("$2");

            base = 0x4F000;
            mul = s6 << 2;
            mul = mul + s6;
            mul = mul << 13;
            s2 = mul + base;
        }
        func_80085EB4((int)s2);
        func_800850F4((int)s0, (int)s1);
        {
            register unsigned int a0r asm("$4");
            register unsigned int a1r asm("$5");
            register unsigned int v asm("$2");

            a0r = (unsigned int)s4;
            a1r = s2;
            v = a1r + s1;
            D_8009D1EC = v;
            v = s5 - s1;
            D_8009D204 = v;
            func_8008A02C((unsigned int *)a0r, a1r, (int)s3);
        }
        {
            register unsigned int *dst asm("$4");
            register unsigned int base asm("$2");

            s2 = s6 << 10;
            base = (unsigned int)D_800B4D00;
            dst = (unsigned int *)(s2 + base);
            s1 = s3 << 4;
            do {
                s1 = s1 - 1;
                w = *s4;
                s4 = s4 + 1;
                *dst = w;
                dst = dst + 1;
            } while (s1 != 0);
        }
        {
            register int mask asm("$4");
            register unsigned int flags asm("$3");
            register unsigned int ret asm("$2");

            mask = -3;
            flags = D_8009D270;
            ret = D_8009D204;
            flags = flags & (unsigned int)mask;
            D_8009D270 = flags;
            return (int)ret;
        }
    }
    s1 = s2;
    func_80085EB4((int)D_8009D1EC);
    if (s1 >= D_8009D204) {
        s1 = D_8009D204;
    }
    func_800850F4((int)s0, (int)s1);
    {
        register unsigned int left asm("$2");
        register unsigned int base asm("$3");

        left = D_8009D204;
        base = D_8009D1EC;
        left = left - s1;
        base = base + s1;
        D_8009D1EC = base;
        D_8009D204 = left;
        return (int)left;
    }
}
