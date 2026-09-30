extern void func_8005267C(void);

int func_800650E0(int *s, int flags)
{
    register int *p asm("$4");
    int ch;
    int done;
    int d;
    int c;
    int n;
    int m;
    int v;
    register int r asm("$4");
    register int e asm("$2");

    p = (int *)s[13];
    done = 0;
    if ((flags & 0x1004) != 0) {
        ch = 0;
        d = -p[14];
        if (p[24] != 0) {
            goto join;
        }
        c = p[23];
        n = c + d;
        p[23] = n;
        if (n < 0) {
            p[23] = 0;
        } else {
            m = p[22] - p[14];
            if (m < n) {
                p[23] = m;
            }
        }
        e = c ^ p[23];
        ch = (e != 0);
        if (ch == 0) {
            done = 1;
            goto out;
        }
        v = p[16];
        if (d <= 0) {
            p[24] = -v / 2;
        } else {
            p[24] = v / 2;
        }
        goto join;
    }
    if ((flags & 0x4008) != 0) {
        ch = 0;
        d = p[14];
        if (p[24] != 0) {
            goto join;
        }
        c = p[23];
        n = c + d;
        p[23] = n;
        if (n < 0) {
            p[23] = 0;
        } else {
            m = p[22] - p[14];
            if (m < n) {
                p[23] = m;
            }
        }
        e = c ^ p[23];
        ch = (e != 0);
        if (ch == 0) {
            done = 1;
            goto out;
        }
        v = p[16];
        if (d <= 0) {
            p[24] = -v / 2;
        } else {
            p[24] = v / 2;
        }
        goto join;
    }
    goto out;
join:
    if (ch != 0) {
        func_8005267C();
        done = 1;
    } else {
        done = 1;
    }
out:
    r = 0;
    if (done != 0) {
        goto one;
    }
    if ((s[16] & 0x40) != 0) {
        goto ret;
    }
one:
    r = 1;
ret:
    return r;
}
