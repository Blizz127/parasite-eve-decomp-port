extern unsigned int D_8009D1A0;
extern unsigned int D_8009D280;
extern int D_800A7918;
extern void func_8006E2D0(void *a0, unsigned int a1);
extern void func_8006A25C(void);

int func_80017BB4(unsigned int **a0) {
    int buf[2];
    int *q;
    unsigned int x;

    func_8006E2D0(buf, **a0);
    x = **a0;
    D_8009D1A0 |= 0x2000;
    D_8009D280 = x;
    if (**a0 == 0xA9400048) {
        q = &D_800A7918;
        if (*q == 0x7D0) {
            *q = 0;
        } else {
            func_8006A25C();
        }
    }
    return 0;
}
