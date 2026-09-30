extern unsigned int D_8009D2C0;
extern unsigned char D_8009D1C8;
extern unsigned char D_8009D1C9;
extern unsigned char D_8009D1CA;
extern unsigned char D_8009D1CB;
extern void func_8007A88C(unsigned char *p);

void func_800870F0(unsigned int a0)
{
    unsigned char v;

    if (D_8009D2C0 & 2) {
        v = (a0 * 2903) >> 13;
        D_8009D1CB = v;
        D_8009D1C9 = v;
        D_8009D1CA = v;
        D_8009D1C8 = v;
    } else {
        v = a0;
        D_8009D1CA = v;
        D_8009D1C8 = v;
        D_8009D1CB = 0;
        D_8009D1C9 = 0;
    }
    func_8007A88C(&D_8009D1C8);
}
