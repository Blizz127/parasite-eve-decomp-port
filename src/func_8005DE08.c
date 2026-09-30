extern int D_800A8054;
extern int D_800A804C;

unsigned char *func_8005DE08(int a0)
{
    int *q;
    unsigned char *base;
    unsigned char *p;

    q = &D_800A8054;
    base = (unsigned char *)(q - 11);
    p = base + *q;
    if (a0 >= 0) {
        a0 = base[a0 + D_800A804C];
    } else {
        a0 = 0xF;
    }
    if (a0 != 0) {
        do {
            if (*p++ == 0) {
                a0--;
            }
        } while (a0 > 0);
        return p;
    }
    return 0;
}
