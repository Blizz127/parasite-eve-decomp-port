void func_80070D10(void) {
    register unsigned int *p asm("$8");
    register unsigned int a asm("$11");
    register unsigned int b asm("$12");
    register unsigned int n asm("$13");
    register unsigned int *i1 asm("$9");
    register unsigned int *i2 asm("$10");

    p = (unsigned int *)0x80070E0Cu;
    asm volatile("" : "=r"(p) : "0"(p));
    a = 1;
    p[16] = a;
    a = 2;
    p[15] = a;
    n = 14;
    do {
        a = p[16];
        b = p[15];
        p = p - 1;
        a = a + b;
        p[15] = a;
    } while (n-- != 0);
    i1 = (unsigned int *)0x80070E04u;
    i2 = (unsigned int *)0x80070E08u;
    asm volatile("" : "=r"(i1) : "0"(i1));
    asm volatile("" : "=r"(i2) : "0"(i2));
    a = 0x40;
    b = 0x10;
    *i1 = a;
    *i2 = b;
}
