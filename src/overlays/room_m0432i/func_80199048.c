extern int *D_8009D254;
extern unsigned short D_800942EC;
extern short D_801994F4;
extern short D_801994F6;
extern short D_801994F8;
extern int D_801994FC;
void func_800CE870(int *a0, int a1, short *a2);

short *func_80199048(int a0, int a1, int a2, int a3)
{
    register int *q asm("$4");
    register short *p asm("$6");
    int h;

    if (a0 == 1) {
        h = D_800942EC;
        q = D_8009D254;
        p = &D_801994F4;
        *p = (short)a1;
        D_801994F8 = (short)a3;
        D_801994F6 = (short)h;
        func_800CE870(q, 1, p);
    } else {
        if (a1 <= 0) {
            a1 = 1;
        }
        D_801994FC = a1;
    }
    return &D_801994F4;
}
