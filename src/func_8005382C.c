
extern short *D_8009D048;
extern int D_8009D050;
extern int *D_8009D058;
extern int D_8009D064;
extern short D_800C0E48[];
extern char D_8009D05C[];
extern int func_80052F70(void);

int func_8005382C(int count)
{
    short *p;
    short *tmp;
    short *lim;
    register short *base asm("$7");
    short *end;
    short *q;
    int size;
    register int i asm("$4");

    D_8009D048 = D_800C0E48;
    size = func_80052F70();
    p = D_8009D048;
    D_8009D058 = (int *)D_8009D05C;
    D_8009D050 = size;
    D_8009D064 = 2;
    tmp = p + size - count;
    if (!(p < tmp + 1))
        return -1;
    base = p;
    asm volatile("" : "=r"(base) : "0"(base));
    lim = tmp;
    asm volatile("" : "=r"(lim) : "0"(lim));
    end = lim + 1;
loop:
    if (*p == 0)
        goto zero;
    p++;
    if (p < end)
        goto loop;
zero:
    if (!(p < lim + 1))
        goto cont;
    i = 1;
    if (!(i < count))
        return p - base;
    if (p[1] != 0)
        goto maybe;
    i = 2;
    if (!(i < count))
        return p - base;
    if (p[2] != 0)
        goto maybe;

    q = p + 2;
    asm volatile("" : "=r"(q) : "0"(q));
    asm volatile("" : "=r"(i) : "0"(i));
    i++;
    q++;
    while (i < count && *q == 0) {
        i++;
        q++;
    }
maybe:
    if (i < count)
        goto cont;
    asm volatile("" : : "r"(i));
    return p - base;
cont:
    p++;
    if (p < lim + 1) {
        end = lim + 1 - count + count;
        goto loop;
    }
    return -1;
}
