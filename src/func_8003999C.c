extern unsigned int D_8009D26C;
extern unsigned int D_8009D1F4;
extern unsigned int D_8009D1E4;

void func_8003999C(int actor, unsigned int **table, unsigned int *codep)
{
    unsigned int *rec;
    unsigned int *q;
    unsigned int flags;
    void (*fn)(int, unsigned int *);
    int matched;
    unsigned int held;
    unsigned int mask2;
    unsigned int bits;
    void (*tail)(int, unsigned int *);

    matched = 0;
    rec = (unsigned int *)table[*codep];
    flags = 0;
    if (rec != 0) {
        flags = rec[0];
    }
    if (flags != 0) {
        q = rec + 2;
        do {
            fn = (void (*)(int, unsigned int *))q[1];
            if (fn == 0) {
                return;
            }
            if ((flags & 0x20) != 0) {
                fn(actor, codep);
                matched = 1;
            } else if ((flags & 1) != 0) {
                held = D_8009D26C;
                if ((held & q[-1]) == q[-1]) {
                    if ((flags & 4) != 0) {
                        if ((D_8009D1F4 & q[0]) != 0) {
                            fn(actor, codep);
                            matched = 1;
                        }
                    } else if ((flags & 8) != 0) {
                        if ((D_8009D1E4 & q[0]) != 0) {
                            fn(actor, codep);
                            matched = 1;
                        }
                    } else if ((flags & 2) != 0) {
                        if ((held & q[0]) != 0) {
                            fn(actor, codep);
                            matched = 1;
                        }
                    } else {
                        fn(actor, codep);
                        matched = 1;
                    }
                }
            } else if ((flags & 2) != 0) {
                bits = D_8009D26C;
                mask2 = q[0];
                if ((bits & mask2) != 0) {
                    if ((flags & 4) != 0) {
                        if ((D_8009D1F4 & mask2) != 0) {
                            fn(actor, codep);
                            matched = 1;
                        }
                    } else if ((flags & 8) != 0) {
                        if ((D_8009D1E4 & mask2) != 0) {
                            fn(actor, codep);
                            matched = 1;
                        }
                    } else {
                        fn(actor, codep);
                        matched = 1;
                    }
                }
            } else if ((flags & 4) != 0) {
                if ((D_8009D1F4 & q[0]) != 0) {
                    fn(actor, codep);
                    matched = 1;
                }
            } else if ((flags & 8) != 0) {
                if ((D_8009D1E4 & q[0]) != 0) {
                    fn(actor, codep);
                    matched = 1;
                }
            }
            if (((flags & 0x10) == 0) && (matched != 0)) {
                return;
            }
            rec += 4;
            flags = rec[0];
            q += 4;
        } while (flags != 0);
    }
    if (rec != 0) {
        tail = (void (*)(int, unsigned int *))rec[3];
        if (tail != 0) {
            tail(actor, codep);
        }
    }
}
