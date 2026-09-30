extern unsigned char D_800BCFFA;
extern unsigned char D_800BCFFB;
extern unsigned int D_800BCF88;
int func_80067B40(unsigned int a0) {
    register unsigned int *p asm("$2") = &D_800BCF88;
    D_800BCFFA = a0;
    D_800BCFFB = 0;
    *p = (*p & ~0xC00) | 0x400;
    return 0;
}
