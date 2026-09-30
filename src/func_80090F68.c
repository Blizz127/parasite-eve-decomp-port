extern unsigned char *D_8009D2C8;
extern unsigned int D_8009D22C;
extern unsigned int D_8009D2C4;
extern void func_8008F1B0(unsigned char *a0, unsigned int a1);
extern void func_80089960(void);
extern void func_80089B28(void);
extern void func_80089CF0(void);

void func_80090F68(unsigned char *a0, unsigned int a1) {
    if (*(unsigned short *)(a0 + 0x54) == 0) {
        unsigned char *s = D_8009D2C8;
        unsigned int t;
        a1 ^= 0xFFFFFF;
        t = *(unsigned int *)(s + 4) & a1;
        *(unsigned int *)(s + 4) = t;
        if (t == 0) {
            D_8009D22C = 0;
            *(unsigned short *)(s + 0x54) = 0;
        }
        s = D_8009D2C8;
        *(unsigned int *)(s + 8) &= a1;
        *(unsigned int *)(s + 0xC) &= a1;
        *(unsigned int *)(s + 0x34) &= a1;
        *(unsigned int *)(s + 0x38) &= a1;
        *(unsigned int *)(s + 0x3C) &= a1;
        if (*(unsigned int *)(a0 + 0x38) & 0x800) {
            *(unsigned int *)(s + 0x30) &= ~(1 << *(unsigned short *)(a0 + 0x5C));
        }
    } else {
        func_8008F1B0(a0, a1);
    }
    *(unsigned int *)(a0 + 0x38) = 0;
    D_8009D2C4 |= 0x10;
    func_80089960();
    func_80089B28();
    func_80089CF0();
}
