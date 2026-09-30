extern int *D_8009D154;
extern int *D_8009D158;
extern void func_800527C0(int a0);
extern int *func_8005DAB4(void);
extern void func_80063E0C();
extern void func_80064D08(int *a0);
extern void func_80064AC0(int *a0);

int *func_8006322C(int a0, int a1, int *a2)
{
    int *cfg;
    int *s;
    int *nx;
    int *pv;
    int *q;
    register int *p asm("$3");
    register int i asm("$4");
    register int *w asm("$2");
    register int t0 asm("$2");
    register int t1 asm("$3");
    register int x0 asm("$19");
    register int x1 asm("$20");
    register int *x2 asm("$18");

    x0 = a0;
    x1 = a1;
    x2 = a2;
    cfg = func_8005DAB4();
    if (cfg == 0) {
        func_800527C0(0xE);
    }
    s = D_8009D158;
    if (s == 0) {
        func_800527C0(0xA);
    }
    nx = (int *)s[0];
    pv = D_8009D154;
    D_8009D154 = s;
    s[1] = x1;
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
    if (x2 != 0) {
        i = 0;
        p = x2;
        __asm__ __volatile__("" : "=r"(p) : "0"(p));
        for (; i < 4; i++) {
            if (p[2] == 0) {
                break;
            }
            p++;
        }
        if (i < 4) {
            w = (int *)(i * 4 + (unsigned int)x2);
            w[2] = (int)s;
        } else {
            func_800527C0(0xB);
        }
    }
    if (s == 0) {
        func_800527C0(0xF);
    }
    s[8] = 2;
    s[28] = x0;
    s[9] = x0;
    t0 = cfg[0];
    s[6] = t0;
    t1 = cfg[1];
    t0 = (int)func_80063E0C;
    s[11] = t0;
    s[7] = t1;
    t0 = cfg[2];
    s[21] = t0;
    s[13] = t0;
    t0 = cfg[3];
    s[27] = t0;
    s[14] = t0;
    t0 = cfg[4];
    s[22] = t0;
    t0 = cfg[5];
    s[15] = t0;
    t1 = cfg[6];
    t0 = -1;
    s[18] = 0;
    s[17] = 0;
    s[20] = t0;
    s[19] = t0;
    s[23] = 0;
    s[24] = 0;
    s[16] = t1;
    t0 = cfg[7];
    s[31] = 0;
    s[30] = 0;
    s[33] = 0;
    s[34] = 0;
    s[26] = 0;
    s[25] = t0;
    s[35] = 0;
    t0 = s[14];
    t1 = s[22];
    s[32] = 0;
    if (t0 < t1) {
        func_80064D08(s);
    }
    func_80064AC0(s);
    return s;
}
