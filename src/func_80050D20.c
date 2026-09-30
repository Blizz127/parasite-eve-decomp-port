extern int D_8009CF40;
extern int D_800A18FC[];
extern int D_800A18D8[];
extern void func_8005E8A4(int, int);
extern void func_8005EB64(int);
extern void func_8005B91C(int, int, int *, int);
extern void func_800605F8(int);

void func_80050D20(int a0)
{
    int tmp;
    int v;

    a0 = a0 + 1;
    v = D_800A18D8[a0] + ((D_8009CF40 * D_800A18FC[a0]) >> 7);
    func_8005E8A4(6, 0);
    func_8005EB64(a0 + 0x8C);
    func_8005E8A4(0x4A, 0);
    func_8005B91C(a0, v, &tmp, 0);
    func_800605F8(tmp + 1);
}
