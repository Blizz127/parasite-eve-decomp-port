extern void *D_800BCDA8;
extern void *D_800BCDAC;
extern void *D_800BCDB0;
extern void *D_800BCDB4;
extern int D_800A1820;
extern int D_800A1824;
extern int D_800A1828;
extern void func_800726F4(void *a0);

void func_800403C8(void) {
    func_800726F4(D_800BCDA8);
    func_800726F4(D_800BCDAC);
    func_800726F4(D_800BCDB0);
    func_800726F4(D_800BCDB4);
    D_800A1828 = 0;
    D_800A1824 = 0;
    D_800A1820 = 0;
}
