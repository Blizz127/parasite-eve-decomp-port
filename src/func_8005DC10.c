extern int D_800A8048;

int func_8005DC10(void) {
    register int *base asm("$2") = &D_800A8048;
    register int value asm("$3");

    value = *base;
    base = (int *)((unsigned char *)base - 0x20);
    return value + (int)base;
}
