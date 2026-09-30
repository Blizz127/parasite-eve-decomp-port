extern int *D_8009D154;
extern int *D_8009D158;
extern void func_80064A54(int *a0);
extern int *func_80062CC4(void);
extern void func_80062CB8(int a0);

void func_8006269C(int *node)
{
    register int *s asm("$16");
    register int *pv asm("$3");
    register int i asm("$17");
    register int *q asm("$18");
    register int *nx asm("$2");
    int t;

    pv = 0;
    s = D_8009D154;
    if (s == 0) {
        return;
    }
loop:
    if (s == node) {
        goto found;
    }
    pv = s;
    s = (int *)s[0];
    if (s != 0) {
        goto loop;
    }
found:
    if (s == 0) {
        return;
    }
    if (pv != 0) {
        pv[0] = s[0];
    } else {
        D_8009D154 = (int *)s[0];
    }
    nx = D_8009D158;
    t = s[8];
    D_8009D158 = s;
    s[0] = (int)nx;
    if (t == 2) {
        func_80064A54(s);
    }
    i = 0;
    q = s;
    do {
        if (q[2] != 0) {
            func_8006269C((int *)q[2]);
        }
        i++;
        q = q + 1;
    } while (i < 4);
    if (func_80062CC4() == s) {
        func_80062CB8(s[1]);
    }
    s = D_8009D154;
    if (s != 0) {
        do {
            if (s[1] == (int)node) {
                s[1] = 0;
            }
            if (s[8] == 2) {
                if (s[30] == (int)node) {
                    s[30] = 0;
                }
                if (s[31] == (int)node) {
                    s[31] = 0;
                }
            }
            i = 0;
            pv = s;
            do {
                if (pv[2] == (int)node) {
                    pv[2] = 0;
                }
                i++;
                pv = pv + 1;
            } while (i < 4);
            s = (int *)s[0];
        } while (s != 0);
    }
}
