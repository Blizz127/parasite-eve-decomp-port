extern int D_80190B8C;
extern int D_80190B90;
extern int D_80190B94;

int *func_80190A90(int a0, int a1, int a2, int a3)
{
    volatile int *p;

    if (a0 == 1) {
        p = &D_80190B8C;
        *p = a1;
        D_80190B90 = a2;
        D_80190B94 = a3;
        *p = 0x1000;
    }
    return &D_80190B8C;
}
