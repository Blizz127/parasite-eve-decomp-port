typedef struct {
    unsigned char c[0xF];
    unsigned char fF;
    int f10;
    int f14;
    unsigned char c18[2];
    unsigned short f1A;
} B;

typedef struct {
    int p0;
    int p4;
    B *f8;
    int pC;
    int p10;
    short f14;
    short f16;
    signed char f18;
} A;

void func_800DFB20(A *a0)
{
    B *b;

    b = a0->f8;
    if (a0->f14 != 0) {
        if (b->f1A >= (unsigned int)b->fF) {
            b->f14 = a0->f18;
            if (a0->f14 != -1) {
                a0->f14 = a0->f14 - 1;
            }
        }
    }
}
