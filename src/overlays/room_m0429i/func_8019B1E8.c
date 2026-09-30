extern short D_8019B66C;
extern short D_8019B66E;

void func_8019B1E8(int a0, int a1, int a2)
{
    short *p;

    p = &D_8019B66C;
    asm volatile("" : "=r"(p) : "0"(p));
    *p = (short)a1;
    D_8019B66E = (short)a2;
}
