extern unsigned char *D_8009D2C8;
extern unsigned char D_800B8AC0[];
extern unsigned char D_800BA560[];
extern void func_8008AB9C(unsigned char *a0);

void func_8008B2CC(unsigned char *a0) {
    int t;
    register int n asm("$6");
    register int val asm("$5");
    register int v asm("$7");
    register int w asm("$2");

    t = *(int *)(a0 + 4);
    n = 1;
    if (t != 0) {
        n = t;
    }
    w = *(int *)(a0 + 8) & 0x7F;
    val = w << 16;
    v = *(int *)(a0 + 0x10);
    if (v == 0 || v == *(unsigned short *)(D_8009D2C8 + 0x54)) {
        unsigned char *s = D_8009D2C8;
        val = (val - *(int *)(s + 0x48)) / n;
        *(unsigned short *)(s + 0x50) = n;
        *(int *)(s + 0x4C) = val;
        func_8008AB9C(D_800B8AC0);
    } else if (a0 != 0 && v == *(unsigned short *)(D_8009D2C8 + 0xBC)) {
        unsigned char *s = D_8009D2C8;
        val = (val - *(int *)(s + 0xB0)) / n;
        *(unsigned short *)(s + 0xB8) = n;
        D_8009D2C8 = s + 0x68;
        *(int *)(s + 0xB4) = val;
        func_8008AB9C(D_800BA560);
        D_8009D2C8 -= 0x68;
    }
}
