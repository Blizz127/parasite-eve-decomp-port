typedef struct {
    int f0;
    int f4;
    int f8;
    short fC;
} Q;

int func_800CE560(Q *a0, int a1, int a2, int a3)
{
    register int i asm("$8");
    register short *p asm("$3");

    p = &a0->fC;
    a1 += 4;
    a0->f8 = a3;
    a0->f0 = a1;
    a0->f4 = a2;
    for (i = 0; i < a2; i++) {
        *p = 0;
        p = (short *)((char *)p + a1);
    }
    return a1 * a2 + 0xC;
}
