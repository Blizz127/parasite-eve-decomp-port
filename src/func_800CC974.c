typedef struct {
    unsigned short a;
    unsigned short b;
    unsigned short c;
    unsigned short d;
} V;

typedef struct {
    char c0;
    unsigned char f1;
    signed char f2;
    char c3;
    short c4;
    unsigned short s06[16];
    V v26[16];
    V vA6[16];
} S;

void func_800CC974(int a0, char *a1, S *a2)
{
    register int i asm("$9");
    register unsigned short t0 asm("$2");
    register int e1 asm("$3");
    register int e2 asm("$4");
    register int e3 asm("$5");
    register int t2b asm("$3");
    int k;
    unsigned char c;

    k = 0;
    i = 0;
    while (i < a2->f2) {
        i++;
        e1 = (short)a2->vA6[k].a >> 9;
        t0 = a2->v26[k].a;
        e2 = (short)a2->vA6[k].b >> 9;
        e3 = (short)a2->vA6[k].c >> 9;
        a2->v26[k].a = t0 + e1;
        t2b = a2->vA6[k].b;
        a2->v26[k].b += e2;
        a2->vA6[k].b = t2b + 0xB4;
        a2->v26[k].c += e3;
        a2->s06[k] += 0x18;
        k++;
    }
    c = a2->f1 - 2;
    a2->f1 = c;
    if (c < 2) {
        a1[1] = 2;
    }
}
