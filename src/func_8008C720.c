extern unsigned char *D_8009D2C8;
extern unsigned int D_800BCD50;
extern unsigned int D_8009D2DC;
extern void func_80087798(int a0, int a1, int a2);
extern void func_800877BC(int a0, int a1);
extern void func_8008780C(int a0, int a1, int a2);
extern void func_8008788C(int a0, int a1, int a2);

void func_8008C720(void) {
    if (*(unsigned int *)(D_8009D2C8 + 4) != 0) {
        unsigned int avail = ~D_800BCD50 & 0xFFFFFF;
        if (avail != 0) {
            unsigned int m = 1;
            int i = 0;
            do {
                if (avail & m) {
                    func_80087798(i, 0, 0);
                    func_800877BC(i, 0);
                    func_8008780C(i, 0x7F, 1);
                    func_8008788C(i, 0x7F, 3);
                    avail &= ~m;
                }
                m <<= 1;
                i++;
            } while (avail != 0);
        }
        {
            unsigned char *s = D_8009D2C8;
            unsigned int t = *(unsigned int *)(s + 4);
            *(unsigned int *)(s + 4) = 0;
            *(unsigned int *)(s + 0x1C) = t;
        }
    }
    D_8009D2DC |= 1;
}
