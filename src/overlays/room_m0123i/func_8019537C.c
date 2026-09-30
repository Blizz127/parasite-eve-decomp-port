extern int D_80195594;
extern int D_801955B4;
extern int *D_801956B8;

int **func_8019537C(int a0, unsigned int a1)
{
    int *p;

    if (a1 >= 2) {
        a1 = 0;
    }
    switch (a1) {
    case 0:
        p = &D_80195594;
        break;
    case 1:
        p = &D_801955B4;
        break;
    default:
        return &D_801956B8;
    }
    D_801956B8 = p;
    return &D_801956B8;
}
