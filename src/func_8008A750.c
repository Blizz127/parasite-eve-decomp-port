extern unsigned int D_800BCD50;
extern unsigned int D_800BCD54;
extern unsigned int D_800BCD58;
extern unsigned int D_800BCD5C;
extern unsigned int D_800BCD60;
extern unsigned int D_800BCD6C;
extern unsigned int D_800BCD70;
extern unsigned int D_800BCD74;
extern unsigned int D_8009D2DC;
extern unsigned char D_800BC000[];
extern void func_80089F58();

void func_8008A750(unsigned char *a0, unsigned char *s, unsigned int m, int a3)
{
    register unsigned char *v asm("$8");
    register unsigned int mask asm("$16");
    register unsigned int *b asm("$4");
    register unsigned int *q asm("$6");
    register unsigned int bit asm("$7");
    int w;
    unsigned int x;
    unsigned char *p;
    int c;
    unsigned int r0;
    unsigned int r1;

    v = a0;
    mask = m;
    *(int *)(v + 0x28) = *(int *)(s + 4);
    *(int *)(v + 0x2C) = *(int *)(s + 8);
    __asm__ volatile ("" : "=r"(v), "=r"(mask) : "0"(v), "1"(mask));
    x = s[0xC];
    *(unsigned short *)(v + 0x78) = 0;
    *(unsigned short *)(v + 0x76) = x << 8;
    w = *(volatile int *)(s + 0x10);
    *(unsigned short *)(v + 0x56) = 2;
    *(unsigned short *)(v + 0x58) = 1;
    *(unsigned short *)(v + 0x54) = 1;
    {
        int neg;
        neg = -2;
        *(unsigned short *)(v + 0x74) = 0;
        *(int *)(v + 0x50) = neg;
    }
    *(unsigned short *)(v + 0xD8) = (w & 0x7F) << 8;
    func_80089F58(a0, a3);
    b = &D_800BCD50;
    r0 = *b;
    r1 = D_800BCD5C;
    r0 |= mask;
    r1 |= mask;
    *b = r0;
    r0 = D_800BCD54;
    mask = ~mask;
    D_800BCD5C = r1;
    r1 = D_800BCD58;
    r0 &= mask;
    D_800BCD54 = r0;
    r0 = D_800BCD6C;
    r1 &= mask;
    D_800BCD58 = r1;
    r1 = D_800BCD70;
    r0 &= mask;
    D_800BCD6C = r0;
    r0 = D_800BCD74;
    r1 &= mask;
    D_800BCD70 = r1;
    r1 = D_8009D2DC;
    r0 &= mask;
    r1 &= 2;
    D_800BCD74 = r0;
    if (r1) {
        mask = 0x1000;
        p = D_800BC000;
        c = 12;
        bit = 0x2000000;
        __asm__ volatile ("" : "=r"(bit) : "0"(bit));
        q = b;
        __asm__ volatile ("" : "=r"(q) : "0"(q));
        do {
            if ((*(unsigned int *)(p + 0x2C) & bit) == 0) {
                register unsigned int inv asm("$2");
                register unsigned int cur asm("$3");
                unsigned int bits;
                inv = ~mask;
                cur = *q;
                bits = D_800BCD60;
                cur &= inv;
                bits |= mask;
                *q = cur;
                D_800BCD60 = bits;
            }
            c -= 1;
            p += 0x11C;
            mask <<= 1;
        } while (c != 0);
    }
}
