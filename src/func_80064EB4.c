extern int *D_8009D154;
extern int *D_8009D15C;
extern int D_8009D10C;
extern volatile int D_8009D124;
extern volatile int D_8009D128;
extern int *D_8009D12C;
extern int D_800A22B0[16];
extern int D_800A2270[16];
extern void func_800527C0(int a0);
extern int func_80073A44(int a0);
extern void func_80062090(int a0, int a1, int a2);
extern void func_80061C34(int a0, int a1, int a2, int a3);
extern void func_8005EB64(int a0);

void func_80064EB4(int *a0)
{
    int *s;
    int *c;
    int *b;
    int *prev;
    register int *sp asm("$5");
    register int *p asm("$3");
    register int i asm("$4");
    register int lt asm("$2");
    register int found asm("$4");
    int tx;
    int ty;
    int d;
    int *q2;
    register int *sp2 asm("$3");
    register int *cur asm("$4");
    register int pa asm("$2");
    register int pb asm("$4");
    register int ua asm("$2");
    register int ub asm("$3");

    s = a0;
    c = (int *)s[13];
    if (c == 0) {
        return;
    }
    prev = 0;
    b = D_8009D154;
    while (b != 0) {
        i = 0;
        p = b;
        for (; i < 4; i++) {
            if (p[2] == (int)s) {
                break;
            }
            p++;
        }
        lt = i < 4;
        if (lt != 0) {
            prev = b;
            break;
        }
        b = (int *)b[0];
    }
    found = 0;
    if (prev[8] == 1) {
        lt = prev[15];
        found = lt != 0;
    }
    sp = D_8009D12C;
    q2 = sp + 2;
    D_8009D10C = found;
    if ((unsigned int)sp < (unsigned int)D_800A22B0) {
        pa = D_8009D124;
        pb = D_8009D128;
        D_8009D12C = q2;
        sp[0] = pa;
        sp[1] = pb;
    } else {
        func_800527C0(2);
    }
    d = c[15] * c[13];
    cur = D_8009D15C;
    ua = s[14];
    ub = D_8009D128;
    ua = ua + 2;
    ub = ub + ua;
    ua = D_8009D124;
    D_8009D128 = ub;
    ub = d + 2;
    ua = ua + ub;
    D_8009D124 = ua;
    if (cur == s) {
        if ((func_80073A44(-1) & 8) != 0) {
            func_80062090(8, s[15], 0);
        }
    }
    func_80061C34(8, s[15], 0, 1);
    if (c[23] != 0) {
        ua = D_8009D128;
        ub = D_8009D124;
        ua = ua - 6;
        D_8009D124 = ub;
        D_8009D128 = ua;
        __asm__ __volatile__("" ::: "memory");
        func_8005EB64(0x4A);
        ua = D_8009D128;
        ub = D_8009D124;
        ua = ua + 6;
        D_8009D124 = ub;
        D_8009D128 = ua;
    }
    if (c[23] < c[22] - c[14]) {
        ua = D_8009D124;
        D_8009D124 = ua;
        ua = s[15];
        ub = D_8009D128;
        ua = ua + 2;
        ub = ub + ua;
        D_8009D128 = ub;
        __asm__ __volatile__("" ::: "memory");
        func_8005EB64(0x4B);
    }
    sp2 = D_8009D12C;
    if ((unsigned int)D_800A2270 < (unsigned int)sp2) {
        tx = sp2[-2];
        ty = sp2[-1];
        D_8009D12C = sp2 - 2;
        D_8009D124 = tx;
        D_8009D128 = ty;
    } else {
        func_800527C0(3);
    }
}
