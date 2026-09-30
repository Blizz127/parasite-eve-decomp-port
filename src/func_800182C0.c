extern unsigned int D_800BCF88;

int func_800182C0(void) {
    register unsigned int *ptr asm("$3") = &D_800BCF88;

    asm volatile("" : "=r"(ptr) : "0"(ptr));
    *ptr |= 0xC0;
    return 1;
}
