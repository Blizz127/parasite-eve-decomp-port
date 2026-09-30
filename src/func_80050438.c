extern void func_8005E8C4();
extern void func_8005E968(int);
extern void func_8005F5B8(int);
extern void func_8005E8A4(int, int);
extern int func_800614A0();
extern void func_8005FB74(int);
extern void func_8005E914();

void func_80050438(int a0) {
    int sh;

    func_8005E8C4();
    sh = a0 * 8;
    func_8005E968(0x80 << sh);
    func_8005F5B8(a0 + 0x35);
    func_8005E968(0x808080);
    func_8005E8A4(0xA, 3);
    func_8005FB74((((func_800614A0() >> sh) & 0xFF) - 0x20) >> 1);
    func_8005E914();
}
