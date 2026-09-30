typedef struct {
    int f0;
    int f4;
    void (*f8)();
    short fC;
} Q;

extern int D_800E27EC;
extern unsigned char *D_800E2368;

int func_800CE78C(Q *a0)
{
    int save;
    void (*fn)();
    int stride;
    short *p;
    int n;
    int i;

    save = D_800E27EC;
    p = &a0->fC;
    i = 0;
    fn = a0->f8;
    stride = a0->f0;
    n = 0;
    for (; i < a0->f4; i++, p = (short *)((char *)p + stride)) {
        if (*p != 0) {
            n++;
            D_800E27EC = p[1];
            fn(2, p + 2, *(int *)(D_800E2368 + 8));
        }
    }
    D_800E27EC = save;
    return n;
}
