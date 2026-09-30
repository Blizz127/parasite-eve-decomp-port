extern int D_8009CEFC;
extern int func_80055FE0(int);
extern void func_8005EB58(int);
extern void func_800536B8(int);
extern int func_80054240(int);
extern void func_80064C80(void);

void func_80050804(int a0)
{
    int flag;

    flag = 0;
    if (D_8009CEFC != 0 || func_80055FE0(a0) == 0) {
        flag = 1;
    }
    func_8005EB58(flag);
    func_800536B8(a0);
    if (func_80054240(a0) != 0) {
        func_80064C80();
    }
}
