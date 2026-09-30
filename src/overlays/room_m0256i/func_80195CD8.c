extern int D_80195EF0;
extern int D_80195EF4;

int *func_80195CD8(int a0, int a1)
{
    if (a1 < 0x200) {
        a1 = 0x200;
    }
    D_80195EF0 = a1;
    return &D_80195EF4;
}
