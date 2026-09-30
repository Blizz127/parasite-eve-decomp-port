extern unsigned int D_800B0CD8;

int func_80019638(void) {
    register unsigned int *ptr asm("$2") = &D_800B0CD8;
    register unsigned int mask asm("$4") = ~0x2000U;

    *ptr &= mask;
    return 1;
}
