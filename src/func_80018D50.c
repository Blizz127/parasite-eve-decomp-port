extern int D_8009D1D8;
extern unsigned int *D_8009D1FC;

int func_80018D50(int **a0) {
    register unsigned char *base asm("$3");

    if (D_8009D1D8 == 0) {
        base = (unsigned char *)(**a0 * 22 + D_8009D1FC[7]);
    } else {
        base = (unsigned char *)(**a0 * 28 + D_8009D1FC[7]);
    }
    *base |= 0x80;
    return 1;
}
