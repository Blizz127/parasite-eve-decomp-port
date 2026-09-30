extern int D_8009D124;
extern int D_8009D128;
extern void func_800602D0(int a, int b);
void func_800605F8(int a) {
    volatile int *p = (volatile int *)&D_8009D128;
    func_800602D0(a, 2);
    D_8009D124 += 2;
    *p = *p;
}
