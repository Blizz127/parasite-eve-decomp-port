extern int D_8019990C;
extern int D_80199910;

void func_80199234(int a0, int a1, int a2)
{
    int *p;

    p = &D_8019990C;
    asm volatile("" : "=r"(p) : "0"(p));
    *p = a2;
    D_80199910 = a1;
}
