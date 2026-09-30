extern unsigned int D_8009D1A0;
extern void func_800661A4(void);
extern void func_8006F8EC(int a0);
extern void func_800661CC(void);
extern void func_8006F9F0(int a0);

int func_80069660(void)
{
    int i;

    if ((D_8009D1A0 & 0x80) != 0) {
        func_800661A4();
        for (i = 0; i < 0xB; i++) {
            func_8006F8EC(i + 0xB);
        }
        func_800661CC();
        if ((D_8009D1A0 & 0x104) != 0) {
            return 0;
        }
        for (i = 0; i < 0xB; i++) {
            func_8006F9F0(i + 0xB);
        }
    }
    return 0;
}
