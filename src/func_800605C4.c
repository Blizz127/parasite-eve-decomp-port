extern int D_8009D124;
extern int D_8009D128;
extern void func_800602D0(int a, int b);
void func_800605C4(int a) {
    volatile int *p = (volatile int *)&D_8009D128;
    func_800602D0(a, 3);
    D_8009D124 += 3;
    *p = *p;
}
