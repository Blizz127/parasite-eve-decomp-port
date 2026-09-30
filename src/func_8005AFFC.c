extern unsigned char *(*D_8009D0B4)(int);
extern unsigned char *D_8009D0B8;
extern unsigned char *D_8009D0BC;
extern unsigned char *func_8005332C();

int func_8005AFFC(short *a, short *b)
{
    unsigned char *(*f)(int);
    int ia;
    int ib;
    unsigned char *p;
    unsigned char *q;
    int ka;
    int kb;
    int la;
    int lb;

    ia = *a;
    ib = *b;
    f = D_8009D0B4;
    if (func_8005332C != f) {
        if (ia == 0) {
            goto ret_ib;
        }
        if (ib == 0) {
            return -1;
        }
    }
    p = f(ia);
    q = D_8009D0B4(ib);
    ka = D_8009D0B8[p[6]];
    kb = D_8009D0B8[q[6]];
    la = D_8009D0BC[p[0xE] & 0xF];
    lb = D_8009D0BC[q[0xE] & 0xF];
    if (p != 0) {
        if (q == 0) {
            goto ret_m1;
        }
    } else {
        if (q != 0) {
            goto one;
        }
    }
    if (kb < ka) {
        return 1;
    }
    if (ka < kb) {
        return -1;
    }
    if (lb < la) {
        return 1;
    }
    if (la < lb) {
        return -1;
    }
    if (p[4] > q[4]) {
        goto one;
    }
    return -(p[4] < q[4]);
one:
    return 1;
ret_m1:
    return -1;
ret_ib:
    return ib != 0;
}
