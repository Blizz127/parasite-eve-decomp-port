extern unsigned int *D_8009D2F0;
extern unsigned int *D_8009D300;
extern int D_8009CE00;

int func_80017540(void) {
    if ((D_8009D2F0[0x26] & 2) == 0) {
        return 1;
    }
    D_8009CE00 -= 8;
    D_8009D300[4] = 1;
    return 0;
}
