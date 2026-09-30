extern char D_8019A831;
extern short D_8019A82A;
extern short D_8019A82C;

char *func_80199D2C(int a0, int a1, int a2, int a3)
{
    char *p;

    p = &D_8019A831;
    *p = (char)a1;
    D_8019A82A = (short)a2;
    D_8019A82C = (short)a3;
    return p - 0x29;
}
