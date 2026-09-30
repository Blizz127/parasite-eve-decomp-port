typedef struct {
    char c0;
    unsigned char f1;
    char c2;
    unsigned char f3;
    short f4;
    short f6;
    short f8;
    short fA;
} S;

extern unsigned short D_800E2808;
extern unsigned short D_800E280A;
extern unsigned short D_800E280C;
extern int func_80071A54(void);

void func_800CE2B4(int a0, int a1, S *a2)
{
    register int m asm("$2");
    register int t asm("$5");

    m = func_80071A54() % 80;
    t = D_800E2808 - 0x28;
    t = t + m;
    a2->f6 = t;
    m = func_80071A54() % 80;
    t = D_800E280A - 0x28;
    t = t + m;
    a2->f8 = t;
    m = func_80071A54() % 80;
    t = D_800E280C - 0x28;
    a2->f1 = 0x7F;
    a2->f4 = 0;
    a2->f3 = 0;
    t = t + m;
    a2->fA = t;
}
