extern int D_8009CFE8;
extern int D_8009CFEC;
extern int D_8009CFF0;
extern int D_8009CF78;
extern void func_80052764(void);
extern int *func_8005DBF8(void);
extern int func_8005B8A8(int, int *);
extern int func_80062A34(int, int);
extern void func_8004BC80(void);
extern void func_80062F1C(int);
extern void func_8005E8A4(int, int);
extern void func_8005E8C4(void);
extern int func_8005DC4C(int);
extern void func_8005F27C(int);
extern void func_80060528(int);
extern void func_8005EB64(int);
extern void func_8005E914(void);
extern int func_80057ECC(void);
extern void func_800605F8(int);

void func_8004B970(void)
{
    int inc;
    int n;
    int rem;

    inc = D_8009CFE8 < D_8009CFEC;
    if (!inc) {
        func_80052764();
    }
    n = func_8005B8A8(D_8009CFE8, func_8005DBF8());
    if (n < 98) {
        rem = func_8005DBF8()[n + 1] - D_8009CFE8;
    } else {
        rem = 0;
    }
    D_8009CFE8 += inc;
    if (D_8009CFF0 < n) {
        D_8009CFF0 = n;
        D_8009CF78 = 60;
        if (func_80062A34(1, 22) == 0) {
            func_8004BC80();
        }
    }
    if (D_8009CF78 != 0) {
        if (--D_8009CF78 == 0) {
            func_80062F1C(func_80062A34(1, 22));
        }
    }
    func_8005E8A4(4, 4);
    func_8005E8C4();
    func_8005F27C(func_8005DC4C(9));
    func_8005E8A4(80, 0);
    func_80060528(D_8009CFE8 > 999999 ? 999999 : D_8009CFE8);
    func_8005E8A4(2, 2);
    func_8005EB64(138);
    func_8005E914();
    func_8005E8A4(0, 14);
    func_8005E8C4();
    func_8005F27C(func_8005DC4C(10));
    func_8005E8A4(80, 0);
    func_80060528(rem);
    func_8005E8A4(2, 2);
    func_8005EB64(138);
    func_8005E914();
    func_8005E8A4(0, 20);
    func_8005E8C4();
    func_8005F27C(func_8005DC4C(12));
    func_8005E8A4(116, 0);
    func_800605F8(func_80057ECC());
    func_8005E914();
}
