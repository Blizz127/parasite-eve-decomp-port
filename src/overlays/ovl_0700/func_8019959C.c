int func_8019959C(unsigned char *base, int idx)
{
    register int off asm("$5");
    register unsigned char *p asm("$2");

    off = idx << 2;
    p = base + off;
    asm volatile("" : "=r"(p) : "0"(p));
    p = base + *(int *)p;
    return *(int *)(p + 0x30);
}
