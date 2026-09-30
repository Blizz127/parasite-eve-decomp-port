extern int D_8009CEF0;
extern int D_8009CEF4;
extern int func_8005B89C(int);
extern int func_80063428(int);
extern int func_80052534(void);
extern void func_8005EB58(int);
extern void func_8005E8A4(int, int);
extern void func_8005EB64(int);

void func_80050748(int a0)
{
    int mask;
    int m;
    int j;
    int n;

    m = a0;
    if (func_8005B89C(a0) != 0) {
        mask = D_8009CEF0 & 0x1F;
    } else {
        mask = D_8009CEF0 & 0x1EF;
    }
    j = -1;
    while (j < 9 && m >= 0) {
        m -= mask & 1;
        j++;
        mask >>= 1;
    }
    n = j;
    if (a0 != func_80063428(D_8009CEF4) || (n == 4 && func_80052534() == 0)) {
        func_8005EB58(1);
    }
    func_8005E8A4(-2, -2);
    func_8005EB64(n + 0x56);
}
