extern int D_8009D030;
extern int D_800B0CD8[];
extern void func_80042798(void);

void func_8005C1EC(int a0)
{
    if (a0 == 0) {
        register int m asm("$4");
        register int *p asm("$3");

        D_8009D030 = 0;
        func_80042798();
        m = 0xFFFF3FFF;
        p = D_800B0CD8;
        *p = *p & m;
        return;
    }
    if (D_8009D030 == 0) {
        register int *q asm("$2");
        register int one asm("$4");
        int v;

        one = 1;
        q = D_800B0CD8;
        v = *q;
        D_8009D030 = one;
        *q = v | 0xC000;
    }
}
