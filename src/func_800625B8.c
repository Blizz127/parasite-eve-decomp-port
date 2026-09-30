extern int *D_8009D154;
extern int *D_8009D158;
extern void func_800527C0(int a0);

int *func_800625B8(int a0, int *a1)
{
    int *s;
    int *nx;
    int *pv;
    int *q;
    register int *p asm("$3");
    register int i asm("$4");
    register int *w asm("$2");

    s = D_8009D158;
    if (s == 0) {
        func_800527C0(0xA);
    }
    nx = (int *)s[0];
    pv = D_8009D154;
    D_8009D154 = s;
    s[1] = a0;
    s[11] = 0;
    s[12] = 0;
    D_8009D158 = nx;
    s[0] = (int)pv;
    i = 3;
    q = s + 3;
    do {
        q[2] = 0;
        i--;
        q--;
    } while (i >= 0);
    s[7] = 0;
    s[6] = 0;
    s[9] = 0;
    s[8] = 0;
    s[10] = 0;
    if (a1 != 0) {
        i = 0;
        p = a1;
        __asm__ __volatile__("" : "=r"(p) : "0"(p));
        for (; i < 4; i++) {
            if (p[2] == 0) {
                break;
            }
            p++;
        }
        if (i < 4) {
            w = (int *)(i * 4 + (unsigned int)a1);
            w[2] = (int)s;
        } else {
            func_800527C0(0xB);
        }
    }
    return s;
}
