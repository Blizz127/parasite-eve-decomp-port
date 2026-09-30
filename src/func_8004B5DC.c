extern void func_8005E8A4(int, int);
extern int func_8005E884();
extern void func_8005FCAC(int);
extern int func_80073A44(int);
extern void func_8005EB58(int);
extern void func_8005EB64(int);
extern void func_8005ED18(int, int);

void func_8004B5DC(void) {
    func_8005E8A4(0x10, 0xA);
    func_8005FCAC(8 - func_8005E884());
    func_8005EB58(func_80073A44(-1) & 0x10);
    func_8005E8A4(-0x10, -0x69);
    func_8005EB64(0x7B);
    func_8005E8A4(0, 0xBE);
    func_8005ED18(0x7B, 2);
}
