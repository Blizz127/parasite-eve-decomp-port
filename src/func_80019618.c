extern unsigned int D_800B0CD8;

int func_80019618(void) {
    register unsigned int *ptr asm("$3") = &D_800B0CD8;

    *ptr |= 0x2000;
    return 1;
}
