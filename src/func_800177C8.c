extern unsigned int *D_8009D300;
extern int D_8009CE00;
extern signed char func_80037548(int a0);

int func_800177C8(short **a0) {
    if (func_80037548(**a0) != 0) {
        D_8009CE00 -= 0xC;
        D_8009D300[4] = 1;
        return 0;
    }
    return 1;
}
