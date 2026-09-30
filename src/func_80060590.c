extern int D_8009D124;
extern int D_8009D128;
extern void func_800602D0(int a, int b);
void func_80060590(int a) {
    volatile int *p = (volatile int *)&D_8009D128;
    func_800602D0(a, 4);
    D_8009D124 += 4;
    *p = *p;
}
