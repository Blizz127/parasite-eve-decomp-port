extern int D_801998F0;
extern int D_801998F4;

void func_80199218(int a0, int a1, int a2)
{
    int *p;

    p = &D_801998F0;
    asm volatile("" : "=r"(p) : "0"(p));
    *p = a2;
    D_801998F4 = a1;
}
