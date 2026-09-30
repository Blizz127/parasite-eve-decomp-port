extern int D_800A8040;

int func_8005DBF8(void) {
    register int *base asm("$2") = &D_800A8040;
    register int value asm("$3");

    value = *base;
    base = (int *)((unsigned char *)base - 0x18);
    return value + (int)base;
}
