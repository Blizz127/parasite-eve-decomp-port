void func_80087798(int voice, int a1, int a2)
{
    unsigned short *base;
    register unsigned short *p asm("$4");

    base = (unsigned short *)0x1F801C00;
    asm ("" : "=r" (base) : "0" (base));
    p = (unsigned short *)((unsigned int)(voice << 4) + (unsigned int)base);
    asm ("" : "=r" (p) : "0" (p));
    p[0] = a1 & 0x7FFF;
    p[1] = a2 & 0x7FFF;
}
