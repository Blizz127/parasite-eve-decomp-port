extern int D_8003E60C;

void func_8003E5F0(void) {
    register int *value_ptr asm("$4") = &D_8003E60C;
    register int value asm("$5") = *value_ptr;

    value++;
    *value_ptr = value;
}
