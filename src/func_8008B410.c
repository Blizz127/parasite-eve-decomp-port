extern unsigned char *D_8009D2C8;
extern unsigned char D_800B8AC0[];
extern unsigned char D_800BA560[];
extern void func_8008AB9C(unsigned char *a0);

void func_8008B410(unsigned char *a0) {
    int t;
    register int n asm("$7");
    register int d asm("$6");
    register int base asm("$3");
    register unsigned char *s asm("$5");
    register int w asm("$2");
    int v;

    t = *(int *)(a0 + 4);
    n = 1;
    if (t != 0) {
        n = t;
    }
    v = *(int *)(a0 + 0x10);
    if (v == 0 || v == *(unsigned short *)(D_8009D2C8 + 0x54)) {
        s = D_8009D2C8;
        base = (*(int *)(a0 + 8) & 0x7F) << 16;
        *(int *)(s + 0x48) = base;
        w = *(int *)(a0 + 0xC) & 0x7F;
        d = w << 16;
        d = d - base;
        d = d / n;
        *(unsigned short *)(s + 0x50) = n;
        *(int *)(s + 0x4C) = d;
        func_8008AB9C(D_800B8AC0);
    } else if (v != 0 && v == *(unsigned short *)(D_8009D2C8 + 0xBC)) {
        s = D_8009D2C8;
        base = (*(int *)(a0 + 8) & 0x7F) << 16;
        *(int *)(s + 0xB0) = base;
        w = *(int *)(a0 + 0xC) & 0x7F;
        d = w << 16;
        d = d - base;
        d = d / n;
        *(unsigned short *)(s + 0xB8) = n;
        D_8009D2C8 = s + 0x68;
        *(int *)(s + 0xB4) = d;
        func_8008AB9C(D_800BA560);
        D_8009D2C8 -= 0x68;
    }
}
