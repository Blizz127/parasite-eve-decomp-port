typedef struct {
    char c0;
    signed char f1;
    char c2;
    char c3;
    unsigned short f4;
    unsigned short f6;
} S;

extern int func_80071A54(void);

void func_800CDFE0(int a0, char *a1, S *a2)
{
    int r;
    unsigned short t6;
    signed char t1;
    unsigned short t4;

    r = func_80071A54();
    t6 = a2->f6 - 4;
    t1 = a2->f1 - 3;
    t4 = a2->f4 - 5;
    a2->f6 = t6;
    a2->f1 = t1;
    a2->f4 = t4 + r % 11;
    if (t1 < 3) {
        a1[1] = 2;
    }
}
