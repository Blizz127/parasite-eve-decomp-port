extern unsigned char D_800B2910[];

unsigned int func_8008E840(int a0, unsigned int a1, int a2) {
    unsigned int n;
    register unsigned int q asm("$7");
    register unsigned int e asm("$5");
    register unsigned char *tab asm("$3");
    register unsigned int m asm("$2");
    unsigned int v;
    unsigned char *base;

    n = a1 & 0xFF;
    q = n / 12;
    base = (unsigned char *)(a0 * 0x40);
    tab = D_800B2910;
    base = base + (unsigned int)tab;
    m = n % 12;
    __asm__ __volatile__("" : "=r"(m) : "0"(m));
    v = *(unsigned int *)(base + m * 4);
    e = q;
    if (a2 != 0) {
        v = v + ((v * a2) >> 7);
    }
    if (e >= 7) {
        m = e - 6;
        v <<= m;
    } else if (q < 6) {
        m = 6 - q;
        v >>= m;
    }
    return v & 0xFFFF;
}
