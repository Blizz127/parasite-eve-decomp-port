extern int D_80190B74;
extern int D_80190B78;
extern int D_80190B7C;
extern int D_80190B80;

int *func_80190A44(int a0, int a1, int a2, int a3)
{
    if (a0 == 1) {
        D_80190B74 = a1;
        D_80190B78 = a2;
        D_80190B7C = a3;
    } else if (a0 == 2) {
        D_80190B80 = a1;
    }
    return &D_80190B74;
}
