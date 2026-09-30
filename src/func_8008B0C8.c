extern void func_8008AB1C(int *a0, int *a1, int a2);
extern void func_8008A92C(int *a0, int a1, int a2);

void func_8008B0C8(int *a0)
{
    int x;
    int y;

    func_8008AB1C(&x, &y, a0[1]);
    a0[2] = 0x2000000;
    a0[3] = 0x80;
    a0[4] = 0x7F;
    func_8008A92C(a0, x, y);
}
