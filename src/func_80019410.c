extern unsigned char D_800BCFEE;
extern int D_8009CE00;
extern unsigned char *D_8009D300;
int func_80019410(void) {
    if ((D_800BCFEE & 3) < 2) return 1;
    D_8009CE00 -= 8;
    *(int *)(D_8009D300 + 0x10) = 1;
    return 0;
}
