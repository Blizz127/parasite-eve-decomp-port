extern int func_80058E08(void);
extern int func_80055FE0(int);
extern void func_8005EB58(int);
extern void func_800536B8(int);
extern int func_80054240(int);
extern void func_80064C80(void);

void func_80050690(void)
{
    int x;

    x = func_80058E08();
    func_8005EB58(func_80055FE0(x) == 0);
    func_800536B8(x);
    if (func_80054240(x) != 0) {
        func_80064C80();
    }
}
