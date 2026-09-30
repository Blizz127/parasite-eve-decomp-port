extern unsigned char D_800C0E0A;
extern int D_800C0E00;
extern signed char D_800C0E20;
extern signed char D_800C0E22;

extern void func_8005E8A4(int, int);
extern void func_8005EB64(int);
extern int func_8005BEE8();
extern void func_8005F27C(int);
extern void func_800605C4(int);
extern int *func_8005DBF8();
extern void func_80060528(int);
extern void func_800536B8(int);
extern void func_8005F5B8(int);

void func_80043B0C(void) {
    unsigned char *p;
    int *v;
    int x;

    func_8005E8A4(2, 2);
    func_8005EB64(0x47);
    func_8005E8A4(0x28, 2);
    func_8005F27C(func_8005BEE8());
    func_8005E8A4(0, 0x15);
    func_8005EB64(0x97);
    func_8005E8A4(0x3C, 0);
    p = &D_800C0E0A;
    func_800605C4(*p + 1);
    func_8005E8A4(-0x78, 0x1A);
    func_8005EB64(0x98);
    func_8005E8A4(0x42, 0);
    if (*p < 0x62) {
        v = func_8005DBF8();
        x = v[*p + 1] - D_800C0E00;
    } else {
        x = 0;
    }
    func_80060528(x);
    func_8005E8A4(-0x7C, 0x15);
    func_8005EB64(0x94);
    func_8005E8A4(0, 0x10);
    func_800536B8(D_800C0E20);
    func_8005E8A4(0, 0x10);
    if (D_800C0E22 >= 0) {
        func_800536B8(D_800C0E22);
    } else {
        func_8005F5B8(0x39);
    }
}
