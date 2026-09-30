typedef struct {
    char c0;
    char c1;
    char c2;
    signed char f3;
    short f4;
    short f6;
    short f8;
} P;

void func_800CC92C(int a0, char *a1, P *a2)
{
    a2->f6 += 0;
    a2->f8 -= 0xA;
    a2->f3 -= 2;
    a2->f4 += 0x1E;
    if (a2->f3 < 0x1E) {
        a1[1] = 2;
    }
}
