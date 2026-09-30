extern unsigned char *D_8009D254;
extern volatile short D_8009CE2C;
extern int D_8009D2F8;
extern unsigned short D_8009D264;

extern int func_8001CE88(int a0, int a1, int a2, int a3);
extern void func_8001CBA0(unsigned char *s, int a1, int a2, short a3, int a4, int a5);

void func_8001D170(void) {
    register unsigned char *s asm("$8");
    unsigned short h224;
    unsigned short h26;
    int prod;
    int h42;
    int h4A;
    short r;
    int a;
    int b;
    int c;

    s = D_8009D254;
    h224 = *(unsigned short *)(s + 0x224);
    h26 = *(unsigned short *)(s + 0x26);
    D_8009CE2C = h224;
    prod = (int)h26 * (int)h224;
    asm volatile("" : "=r"(prod) : "0"(prod));
    if (prod < 0) {
        prod += 4095;
    }
    {
        int h2A = *(short *)(s + 0x2A);
        int h32 = *(short *)(s + 0x32);
        int g1 = D_8009D2F8;
        unsigned int g2 = D_8009D264;
        asm volatile("" : "=r"(h2A) : "0"(h2A));
        asm volatile("" : "=r"(h32) : "0"(h32));
        asm volatile("" : "=r"(g1) : "0"(g1));
        asm volatile("" : "=r"(g2) : "0"(g2));
        h42 = *(short *)(s + 0x42);
        h4A = *(short *)(s + 0x4A);
        D_8009CE2C = prod >> 12;
        r = func_8001CE88(h2A, h32, g1, g2);
    }
    if (r < 0) {
        return;
    }
    func_8001CBA0(D_8009D254, D_8009D2F8, D_8009D264, r, h42, h4A);
    if ((func_8001CE88(*(short *)(D_8009D254 + 0x2A), *(short *)(D_8009D254 + 0x32), D_8009D2F8, D_8009D264) << 16) < 0) {
        return;
    }
    {
        unsigned char *q = D_8009D254;
        a = *(int *)(q + 0x40);
        b = *(int *)(q + 0x44);
        c = *(int *)(q + 0x48);
        *(int *)(q + 0x28) = a;
        *(int *)(q + 0x2C) = b;
        *(int *)(q + 0x30) = c;
    }
}
