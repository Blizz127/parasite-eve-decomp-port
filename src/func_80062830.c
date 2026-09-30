extern int *D_8009D12C;
extern int D_8009D124;
extern int D_8009D128;
extern int D_800A22B0[16];
extern int D_800A2270[16];
extern void func_800527C0(int a0);

void func_80062830(int *s)
{
    register int *t asm("$16");
    register int i asm("$17");
    register int *sp asm("$5");
    register int *q asm("$3");
    register int x asm("$2");
    register int y asm("$4");
    register int a asm("$6");
    register int b asm("$5");
    register int h asm("$4");
    register int g asm("$2");
    register int u asm("$3");
    register int v asm("$2");
    register int *n asm("$2");

    t = s;
    sp = D_8009D12C;
    if (sp < D_800A22B0) {
        x = D_8009D124;
        y = D_8009D128;
        D_8009D12C = sp + 2;
        sp[0] = x;
        sp[1] = y;
    } else {
        func_800527C0(2);
    }
    u = t[6];
    v = D_8009D124;
    h = t[12];
    a = v + u;
    u = t[7];
    v = D_8009D128;
    D_8009D124 = a;
    b = v + u;
    D_8009D128 = b;
    i = 0;
    if (h != 0) {
        q = D_8009D12C;
        if (q < D_800A22B0) {
            n = q + 2;
            q[0] = a;
            q[1] = b;
            D_8009D12C = n;
        } else {
            func_800527C0(2);
        }
        g = t[12];
        ((void (*)(int *))g)(t);
        q = D_8009D12C;
        if (D_800A2270 < q) {
            i = 0;
            x = q[-2];
            y = q[-1];
            D_8009D12C = q - 2;
            D_8009D124 = x;
            D_8009D128 = y;
        } else {
            func_800527C0(3);
            i = 0;
        }
    }
    do {
        if (t[2] != 0) {
            func_80062830((int *)t[2]);
        }
        i++;
        t = t + 1;
    } while (i < 4);
    q = D_8009D12C;
    if (D_800A2270 < q) {
        x = q[-2];
        y = q[-1];
        D_8009D12C = q - 2;
        D_8009D124 = x;
        D_8009D128 = y;
    } else {
        func_800527C0(3);
    }
}
