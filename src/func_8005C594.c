extern int D_8009D1E0[];
extern int D_8009D030;
extern int D_8009D034;
extern unsigned char D_8009D02C;
extern int func_80042ED0(void);
extern void func_80042F44(void);
extern void func_80051504(void);
extern void func_8005E6F0(void);
extern void func_80046334(void);
extern void func_8005E30C(void);
extern void func_8004F464(void);
extern void func_80042B6C(void);
extern void func_80062FEC(void);
extern void func_8005E788(int);
extern void func_800425DC(void);
extern void func_800512AC(int, int);
extern int func_800514F8(void);
extern void func_800339A0(int);

int func_8005C594(void)
{
    int n;

    if (func_80042ED0() != 0) {
        goto fail;
    }
    D_8009D1E0[0] = 0;
    func_80051504();
    func_8005E6F0();
    func_80046334();
    func_8005E30C();
    func_8004F464();
    func_80042B6C();
    func_80062FEC();
    func_8005E788(1);
    n = D_8009D030;
    if (n >= 2) {
        func_800425DC();
    } else if (n > 0) {
        D_8009D030 = n + 1;
    }
    if (D_8009D034 != 0) {
        D_8009D034 = 0;
        func_800512AC(9, 0);
    }
    if (func_800514F8() != 0) {
        func_800339A0(D_8009D02C);
    }
    return func_800514F8();
fail:
    func_80042F44();
    return 0;
}
