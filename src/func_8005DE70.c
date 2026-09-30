extern int D_800A8044;

int func_8005DE70(void) {
    register int *base asm("$2") = &D_800A8044;
    register int value asm("$3");

    value = *base;
    base = (int *)((unsigned char *)base - 0x1C);
    return value + (int)base;
}
