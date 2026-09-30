extern unsigned int D_800BCD50;
extern unsigned int D_800BCD60;
extern unsigned int D_8009D2DC;
extern unsigned char D_800BC000[];
extern void func_80087798(int a0, int a1, int a2);
extern void func_800877BC(int a0, int a1);
extern void func_8008780C(int a0, int a1, int a2);
extern void func_8008788C(int a0, int a1, int a2);

void func_8008C8B8(void) {
    unsigned int f = D_800BCD50;

    if (f != 0) {
        unsigned char *p = D_800BC000;
        unsigned int mask = 0x1000;
        unsigned int i = 0;
        unsigned int bit = 0x2000000;
        unsigned int *q;
        int ch;

        do {
            if (f & mask) {
                if (*(unsigned int *)(p + 0x2C) & bit) {
                    f &= ~mask;
                }
            }
            i++;
            p += 0x11C;
            mask <<= 1;
        } while (i < 0xC);
        mask = 0x1000;
        ch = 0xC;
        q = &D_800BCD50;
        D_800BCD60 = f;
        *q &= ~f;
        if (f != 0) {
            do {
                if (f & mask) {
                    func_80087798(ch, 0, 0);
                    func_800877BC(ch, 0);
                    func_8008780C(ch, 0x7F, 1);
                    func_8008788C(ch, 0x7F, 3);
                    f &= ~mask;
                }
                mask <<= 1;
                ch++;
            } while (f != 0);
        }
    }
    D_8009D2DC |= 2;
}
