typedef struct {
    char c0;
    unsigned char f1;
    unsigned char f2;
    char c3;
    char c4[4];
    short f8;
    short fA;
    short fC;
    short cE;
    short f10;
    short f12;
    short f14;
} P;

void func_800CA4B4(int a0, char *a1, P *a2)
{
    register P *p asm("$4");
    unsigned char t;

    p = a2;
    a2->f8 += a2->f10;
    a2->fA += a2->f12;
    a2->fC += a2->f14;
    a2->f12 += 3;
    a2->f1 += 1;
    if (a2->fA > 0) {
        a2->f12 = -a2->f12;
    }
    t = p->f2;
    p->f2 = t - 1;
    if (t == 0) {
        a1[1] = 2;
    }
}
