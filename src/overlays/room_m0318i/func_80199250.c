extern int D_80199914;
extern int D_80199918;
extern int D_8019991C;
extern int D_80199920;

int *func_80199250(int a0, int a1, int a2, int a3)
{
    if (a0 == 1) {
        D_80199914 = a1;
    } else {
        D_80199918 = a1;
        D_8019991C = a2;
        D_80199920 = a3;
    }
    return &D_80199914;
}
