extern unsigned char *D_8009D2C8;
extern unsigned char D_800B8BB4[];
extern unsigned int D_8009D2DC;
extern void func_80089960(void);
extern void func_80089B28(void);
extern void func_80089CF0(void);

void func_8008C814(void) {
    unsigned int m;

    m = *(unsigned int *)(D_8009D2C8 + 0x1C);
    if (m != 0) {
        unsigned int mask = 1;
        unsigned char *p = D_800B8BB4;
        do {
            if (m & mask) {
                m &= ~mask;
                *(unsigned int *)p |= 0x2B13;
            }
            mask <<= 1;
            p += 0x11C;
        } while (m != 0);
        {
            unsigned char *b = D_8009D2C8;
            unsigned int t = *(unsigned int *)(b + 0x1C);
            *(unsigned int *)(b + 0x1C) = 0;
            *(unsigned int *)(b + 4) = t;
        }
        func_80089960();
        func_80089B28();
        func_80089CF0();
    }
    D_8009D2DC &= ~1;
}
