extern volatile int D_800B0E08;
extern int func_8006DED4(int a0, int a1, int a2, int a3, int a4, int a5);

int func_8006DDCC(int a0, int a1, short a2, short a3, short a4)
{
    volatile int *p = &D_800B0E08;
    register int x0 asm("$20");
    register int x1 asm("$21");
    register int x2 asm("$18");
    register int x3 asm("$17");
    register int r asm("$16");

    x0 = a0;
    x1 = a1;
    x2 = a2;
    x3 = a3;
    r = func_8006DED4(*p, x0, x1, x2, x3, a4);
    func_8006DED4(*p, x0 + 1, x1, x2, x3, a4);
    return r;
}
