extern unsigned char *func_80062A20(int, int);
extern int func_80063428(unsigned char *);
extern int func_8005E54C();
extern void func_8005E8A4(int, int);
extern void func_8005EB58(int);
extern void func_8005EB64(int);

void func_8004B214(int a0) {
    int sel;
    int flag;

    sel = func_80063428(func_80062A20(a0, 0));
    flag = func_8005E54C() & 0x1000;
    func_8005E8A4(0x12, 5);
    func_8005EB58((sel == 0 && flag != 0) ? 0 : 1);
    func_8005EB64(0x4A);
    func_8005E8A4(0x20, 0);
    func_8005EB58((sel == 1 && flag != 0) ? 0 : 1);
    func_8005EB64(0x4A);
    func_8005E8A4(0x20, 0);
    func_8005EB58((sel == 2 && flag != 0) ? 0 : 1);
    func_8005EB64(0x4A);
    flag = func_8005E54C() & 0x4000;
    func_8005E8A4(-0x40, 0x19);
    func_8005EB58((sel == 0 && flag != 0) ? 0 : 1);
    func_8005EB64(0x4B);
    func_8005E8A4(0x20, 0);
    func_8005EB58((sel == 1 && flag != 0) ? 0 : 1);
    func_8005EB64(0x4B);
    func_8005E8A4(0x20, 0);
    func_8005EB58((sel == 2 && flag != 0) ? 0 : 1);
    func_8005EB64(0x4B);
}
