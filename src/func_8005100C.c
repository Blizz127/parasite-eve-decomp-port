extern int D_8009CEF4;
extern int func_80063428(int);
extern void func_8005EB58(int);
extern void func_8005E8A4(int, int);
extern void func_8005EB64(int);

void func_8005100C(int a0)
{
    if (a0 != func_80063428(D_8009CEF4)) {
        func_8005EB58(1);
    }
    func_8005E8A4(-2, -2);
    func_8005EB64(a0 + 0x62);
}
