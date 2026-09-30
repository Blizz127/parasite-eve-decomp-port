extern unsigned int D_800BCD50;
extern unsigned int D_800BCD5C;
extern unsigned int D_8009D2C4;
extern unsigned char D_800BC000[];
extern void func_8008F1B0(unsigned char *a0, unsigned int a1);
extern void func_80089960(void);
extern void func_80089B28(void);
extern void func_80089CF0(void);

void func_8008C46C(void) {
    unsigned char *base = D_800BC000;
    unsigned int mask = 0x1000;
    unsigned int i = 0;
    unsigned int bit = 0x2000000;
    unsigned char *p = base + 0x38;

    do {
        if (D_800BCD50 & mask) {
            if ((*(unsigned int *)(p - 0xC) & bit) == 0) {
                D_800BCD5C |= mask;
                func_8008F1B0(base, mask);
                *(unsigned int *)p = 0;
            }
        }
        i++;
        p += 0x11C;
        base += 0x11C;
        mask <<= 1;
    } while (i < 0xC);
    D_8009D2C4 |= 0x10;
    func_80089960();
    func_80089B28();
    func_80089CF0();
}
