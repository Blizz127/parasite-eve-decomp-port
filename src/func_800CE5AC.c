typedef struct {
    int f0;
    int f4;
    int f8;
    short fC;
} Q;

typedef struct {
    int p0[2];
    char *f8;
} G;

extern G *D_800F33E0;

int func_800CE5AC(Q **a0, int a1, int a2, int a3, int a4)
{
    register int i asm("$8");
    Q *q;
    short *p;

    q = (Q *)(D_800F33E0->f8 + a1);
    a2 += 4;
    p = &q->fC;
    *a0 = q;
    q->f8 = a4;
    q->f0 = a2;
    q->f4 = a3;
    for (i = 0; i < a3; i++) {
        *p = 0;
        p = (short *)((char *)p + a2);
    }
    return a2 * a3 + 0xC;
}
