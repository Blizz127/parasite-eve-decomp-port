typedef struct {
    char c0;
    char c1;
    char c2;
    signed char f3;
    short f4;
    short c6;
    unsigned short a08[16];
    unsigned short a28[16];
    unsigned short a48[16];
    unsigned short a68[16];
    unsigned short a88[16];
    unsigned short aA8[16];
} S;

void func_800CCAB0(int a0, char *a1, S *a2)
{
    register int i asm("$8");
    register unsigned short t0 asm("$2");
    register int e1 asm("$3");
    register int e2 asm("$4");
    register int e3 asm("$5");
    register int t2b asm("$3");
    int k;
    signed char c;

    k = 0;
    i = 0;
    while (i < a2->f4) {
        i++;
        e1 = (signed char)(a2->a68[k] >> 8);
        t0 = a2->a08[k];
        e2 = (signed char)(a2->a88[k] >> 8);
        e3 = (signed char)(a2->aA8[k] >> 8);
        a2->a08[k] = t0 + e1;
        t2b = a2->a88[k];
        a2->a28[k] += e2;
        a2->a88[k] = t2b + 0xB4;
        a2->a48[k] += e3;
        k++;
    }
    c = a2->f3 - 2;
    a2->f3 = c;
    if (c < 2) {
        a1[1] = 2;
    }
}
