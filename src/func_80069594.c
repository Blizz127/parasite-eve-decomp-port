extern unsigned int D_8009D1A0;
extern unsigned char *D_800942E4;
extern void func_800661A4(void);
extern void func_8006F8EC(int a0);
extern void func_800661CC(void);
extern void func_8006F9F0(int a0);

int func_80069594(void)
{
    int i;
    unsigned int k;
    unsigned int c;

    if ((D_8009D1A0 & 0x80) != 0) {
        func_800661A4();
        for (i = 0; i < 0xB; i++) {
            func_8006F8EC(i);
        }
        func_800661CC();
        if ((D_8009D1A0 & 4) != 0) {
            return 0;
        }
        i = 0;
        k = 0;
        for (; i < 0xB; i++) {
            c = ((unsigned char *)(k + (unsigned int)D_800942E4))[1];
            if ((D_8009D1A0 & 0x100) == 0 || (c - 0x55) < 0x1E) {
                func_8006F9F0(i);
            }
            k += 0xA0C;
        }
    }
    return 0;
}
