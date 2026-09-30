extern int D_8009CF50;
extern int *D_8009CEF4;

extern unsigned char *func_800424B4(int, int);
extern int func_8003FFBC();

int func_8004FE58(int a0) {
    unsigned char *p;
    int r;

    if (D_8009CF50 != 0) {
        p = func_800424B4(D_8009CEF4[9] - 0x25, a0);
        r = (p[0] != 3);
    } else {
        p = func_800424B4(D_8009CEF4[9] - 0x25, a0);
        if (p[0] != 1) {
            r = 0;
        } else {
            r = p[0x29];
            r = ((r ^ func_8003FFBC()) == 0);
        }
    }
    return r;
}
