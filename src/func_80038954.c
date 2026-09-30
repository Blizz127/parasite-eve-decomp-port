extern char D_80010EB0[];
extern char D_80010EC8[];
extern void *D_80091A2C[];
extern void func_80071A74(char *a0, void *a1, void *a2);

void func_80038954(void *a0, void *a1, void *a2, int a3) {
    unsigned char i;

    i = a3;
    func_80071A74(D_80010EB0, D_80091A2C[i], a0);
    func_80071A74(D_80010EC8, a1, a2);
    if (i == 1) {
        for (;;) {
        }
    }
}
