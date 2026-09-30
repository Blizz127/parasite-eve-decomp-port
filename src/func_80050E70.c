extern int D_800A18EC[];
extern void func_8005E8A4(int, int);
extern void func_8005EB64(int);
extern void func_8005B91C(int, int, int *, int);
extern void func_800605F8(int);
extern void func_800437B4(int);

void func_80050E70(int a0)
{
    int tmp;
    int v;
    int i;

    v = D_800A18EC[a0];
    func_8005E8A4(2, 1);
    func_8005EB64(a0 + 0x91);
    func_8005E8A4(0x42, -1);
    i = a0 + 5;
    func_8005B91C(i, v, &tmp, 0);
    func_800605F8(tmp + 1);
    func_8005E8A4(2, 0);
    func_800437B4(i);
    func_8005E8A4(-0x44, 0xE);
}
