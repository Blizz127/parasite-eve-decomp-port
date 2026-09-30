extern int D_8009D22C;
extern void func_8008AE94(int a0);

void func_8008B040(int a0)
{
    int r;
    int v;

    func_8008AE94(a0);
    r = 0;
    v = ((int *)a0)[4];
    if (v != 0) {
        r = v - 1;
    }
    D_8009D22C = r;
}
