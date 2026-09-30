extern int D_8009CEF4;
extern void func_800638D8(unsigned char *a, void (*f)(void));
extern void func_8005EB58(int a);
extern void func_8005EB64(int a);
extern void func_8005100C(void);
void func_800501C8(unsigned char *a0) {
    D_8009CEF4 = (int)a0;
    func_800638D8(a0, func_8005100C);
    func_8005EB58(1);
    func_8005EB64(0x68);
}
