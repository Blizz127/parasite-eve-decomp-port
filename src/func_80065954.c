extern unsigned char *volatile D_800B1624;

int func_80065954(unsigned int a0, int a1)
{
    register unsigned char *p asm("$3");
    register unsigned char *q asm("$2");

    p = D_800B1624;
    q = D_800B1624;
    q += *(unsigned int *)(p + 0x10);
    q += a0 << 4;
    if (a1 != 0) {
        *q = *q | 6;
    } else {
        *q = *q & 0xF9;
    }
    return 0;
}
