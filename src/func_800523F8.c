extern unsigned short D_800C0E2E;
extern int D_800A1B3C;
extern void func_8005B91C(int, int, int *, int);
extern int *func_8005DBAC(int);

void func_800523F8(int *a0, int *a1)
{
    int tmp;
    int *r;

    func_8005B91C(3, D_800C0E2E, &tmp, 0);
    r = func_8005DBAC(tmp + D_800A1B3C);
    if (a0 != 0) {
        *a0 = r[4];
    }
    if (a1 != 0) {
        *a1 = r[3];
    }
}
