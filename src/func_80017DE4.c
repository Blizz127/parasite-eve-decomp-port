extern int func_80037864(void);
int func_80017DE4(unsigned char *a0) {
    signed char b = (signed char)func_80037864();
    *(unsigned int *)*(unsigned int *)a0 = b;
    return 1;
}
