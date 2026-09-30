extern int D_8009D000;

extern void func_8005E8A4(int, int);
extern void func_80062A7C(int);
extern int func_80042464();
extern void func_8005EB64(int);

void func_8004DA04(void) {
    int i;
    int n;

    func_8005E8A4(0, 0xA);
    func_80062A7C(D_8009D000);
    if (D_8009D000 != 0x40) {
        n = func_80042464();
        func_8005E8A4(0x10, 0x14);
        for (i = 0; i < 8; i++) {
            func_8005EB64(i < n ? 0x83 : 0x82);
            func_8005E8A4(0x10, 0);
        }
    }
}
