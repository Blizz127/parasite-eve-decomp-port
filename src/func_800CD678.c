typedef struct {
    int c0;
    short f4;
    short c6;
    unsigned short f8;
    unsigned short fA;
} S;

extern int func_80071A54(void);

void func_800CD678(int a0, char *a1, S *a2)
{
    int r;
    unsigned short tA;
    short t4;
    unsigned short t8;

    r = func_80071A54();
    tA = a2->fA - 8;
    t4 = a2->f4 - 8;
    t8 = a2->f8 - 5;
    a2->fA = tA;
    a2->f4 = t4;
    a2->f8 = t8 + r % 11;
    if (t4 < 8) {
        a1[1] = 2;
    }
}
