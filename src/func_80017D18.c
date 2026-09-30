extern unsigned int D_800BCF88;

int func_80017D18(void) {
    register unsigned int *value asm("$2") = &D_800BCF88;

    *value = (*value & ~7U) | 0x80U;
    return 1;
}
