extern unsigned int D_800A76C4;

int func_80018754(void) {
    register unsigned int *ptr asm("$3") = &D_800A76C4;

    *ptr |= 4;
    return 1;
}
