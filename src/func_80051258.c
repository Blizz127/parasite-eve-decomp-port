extern unsigned char *D_8009D014;
extern unsigned char D_800A1AA0[];
extern void func_8005112C(void);

void func_80051258(void)
{
    while (D_800A1AA0 < D_8009D014) {
        func_8005112C();
    }
}
