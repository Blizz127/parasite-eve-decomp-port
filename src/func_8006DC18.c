extern int D_800B0E64;
extern unsigned char *func_8006E498(int a0, unsigned int a1);

int func_8006DC18(int a0)
{
    register int a asm("$18");
    register unsigned int key asm("$16");
    register unsigned int m1 asm("$17");
    register int base asm("$19");
    register unsigned int m2 asm("$20");
    register unsigned int m3 asm("$21");
    register unsigned int m4 asm("$22");
    unsigned char *p;
    unsigned int w;

    a = a0;
    key = 0x73DECD80;
    m1 = 0x1FFC00;
    base = D_800B0E64;
    m2 = 0xD0000;
    m3 = 0xFFE00000;
    m4 = 0x20000000;
    for (;;) {
        p = func_8006E498(base, key);
        if (p == 0) {
            return -1;
        }
        if (p[3] == a) {
            w = *(unsigned int *)(p + 8);
            if ((w & m1) == m2) {
                return ((w & m3) ^ m4) != 0;
            }
        }
        key += 4;
    }
}
