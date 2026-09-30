void func_80193114(int a0, int a1, short *a2)
{
    register int one asm("$2");
    register int n asm("$3");

    one = -0x1000;
    n = 0x270;
    asm volatile("" : "=r"(n) : "0"(n));
    a2[1] = (short)one;
    one = 0x1000;
    a2[9] = (short)one;
    one = 1;
    a2[4] = 0;
    a2[5] = 0;
    a2[6] = 0;
    a2[0] = 0;
    a2[2] = 0;
    a2[8] = (short)n;
    a2[10] = (short)n;
    a2[12] = (short)one;
}
