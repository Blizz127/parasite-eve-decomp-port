extern unsigned char D_800C0E0C[];
extern unsigned char *D_8009D048;
extern int D_8009D050;
extern int func_80052F70(void);

void func_80052F24(int a0)
{
    unsigned char *p;

    if (a0 >= 0x33) {
        a0 = 0x32;
    }
    p = D_800C0E0C;
    p[0] = a0;
    if (D_8009D048 == p + 0x3C) {
        D_8009D050 = func_80052F70();
    }
}
