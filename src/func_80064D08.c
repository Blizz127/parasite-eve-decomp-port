extern int *D_8009D154;
extern int *D_8009D158;
extern void func_800527C0(int a0);
extern void func_80064EB4();
extern void func_800650E0();

void func_80064D08(int *a0)
{
    int *owner;
    register int *s asm("$16");
    register int *prev asm("$18");
    register int cls asm("$19");
    register int *p asm("$3");
    register int i asm("$4");
    register int *w asm("$2");
    register int lt asm("$2");
    int *b;
    register int *pv asm("$6");
    int *nx;
    int *ov;
    int *q;

    owner = a0;
    b = D_8009D154;
    pv = 0;
    while (b != 0) {
        i = 0;
        p = b;
        for (; i < 4; i++) {
            if (p[2] == (int)owner) {
                break;
            }
            p++;
        }
        lt = i < 4;
        if (lt != 0) {
            pv = b;
            break;
        }
        b = (int *)b[0];
    }
    prev = pv;
    s = D_8009D158;
    cls = owner[1];
    if (s == 0) {
        func_800527C0(0xA);
    }
    nx = (int *)s[0];
    ov = D_8009D154;
    D_8009D154 = s;
    s[1] = cls;
    s[11] = 0;
    s[12] = 0;
    D_8009D158 = nx;
    s[0] = (int)ov;
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
    if (prev != 0) {
        i = 0;
        p = prev;
        __asm__ __volatile__("" : "=r"(p) : "0"(p));
        for (; i < 4; i++) {
            if (p[2] == 0) {
                break;
            }
            p++;
        }
        if (i < 4) {
            w = (int *)(i * 4 + (unsigned int)prev);
            w[2] = (int)s;
        } else {
            func_800527C0(0xB);
        }
    }
    if (s == 0) {
        func_800527C0(0x10);
    }
    s[8] = 3;
    s[12] = (int)func_80064EB4;
    s[11] = (int)func_800650E0;
    s[13] = (int)owner;
    s[16] = owner[25];
    owner[32] = (int)s;
}
