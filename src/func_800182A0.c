extern unsigned int D_800BCF88;

int func_800182A0(void) {
    register unsigned int *ptr asm("$2") = &D_800BCF88;

    asm volatile("" : "=r"(ptr) : "0"(ptr));
    *ptr &= ~0xC0;
    return 1;
}
