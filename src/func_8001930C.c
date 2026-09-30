extern unsigned char *D_8009D254;
extern unsigned char *D_8009D20C;
extern void func_8006FE14(void *a0);

int func_8001930C(int **a0) {
    unsigned char *p;

    if (**a0 == 0) {
        p = D_8009D254;
    } else {
        p = D_8009D20C;
        while (p != 0) {
            if (p[0xC] == **a0 && p[0xD] == *a0[1] &&
                (*(int *)(p + 0x98) & 0x10) == 0) {
                break;
            }
            p = *(unsigned char **)(p + 4);
        }
    }
    func_8006FE14(p);
    return 1;
}
