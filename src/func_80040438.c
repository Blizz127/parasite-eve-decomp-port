extern void *D_800BCDB8;
extern void *D_800BCDBC;
extern void *D_800BCDC0;
extern void *D_800BCDC4;
extern int D_800A182C;
extern int D_800A1830;
extern int D_800A1834;
extern void func_800726F4(void *a0);

void func_80040438(void) {
    func_800726F4(D_800BCDB8);
    func_800726F4(D_800BCDBC);
    func_800726F4(D_800BCDC0);
    func_800726F4(D_800BCDC4);
    D_800A1834 = 0;
    D_800A1830 = 0;
    D_800A182C = 0;
}
