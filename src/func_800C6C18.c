extern void *D_8009D254;
extern int func_800C6CE0();

int func_800C6C18(unsigned char *a0)
{
    int r;
    int t;
    unsigned char *p;
    unsigned char *q;

    t = func_800C6CE0(a0);
    r = 1;
    if (t == 3) {
        r = *(*(unsigned char **)(**(unsigned char ***)(a0 + 8) + 0x18)) == 2;
    }
    if (r != 0) {
        if (func_800C6CE0(a0) == 3) {
            p = *(unsigned char **)D_8009D254;
            *(unsigned int *)(p + 0x4C) |= 0x4000;
            q = **(unsigned char ***)(a0 + 8);
            *(unsigned int *)q |= 0x80000000;
        }
    }
    return r;
}
