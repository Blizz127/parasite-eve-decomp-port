extern unsigned int D_800BCD60;
extern unsigned int D_800BCD50;
extern unsigned char D_800BC0F4[];
extern unsigned int D_8009D2DC;
extern void func_80089960(void);
extern void func_80089B28(void);
extern void func_80089CF0(void);

void func_8008C9DC(void) {
    unsigned int m;

    m = D_800BCD60;
    if (m != 0) {
        unsigned int mask = 0x1000;
        unsigned char *p = D_800BC0F4;
        do {
            if (m & mask) {
                m &= ~mask;
                *(unsigned int *)p |= 0x2B13;
            }
            mask <<= 1;
            p += 0x11C;
        } while (m != 0);
        {
            unsigned int t = D_800BCD60;
            D_800BCD60 = 0;
            D_800BCD50 = t;
        }
        func_80089960();
        func_80089B28();
        func_80089CF0();
    }
    D_8009D2DC &= ~2;
}
