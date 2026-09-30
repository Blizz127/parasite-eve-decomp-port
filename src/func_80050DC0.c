extern int D_800A1888[];
extern void func_8005E8C4(void);
extern void func_8005F5B8(int);
extern void func_8005E8A4(int, int);
extern void func_8005FB74(int);
extern void func_8005FA3C(int);
extern void func_8005E914(void);

void func_80050DC0(int a0)
{
    func_8005E8C4();
    func_8005F5B8((a0 & 1) | 0x6A);
    if (a0 >= 2 || D_800A1888[a0] >= 0x64) {
        func_8005E8A4(0x28, 3);
        func_8005FB74(D_800A1888[a0]);
    } else {
        func_8005E8A4(0x2D, 3);
        func_8005FA3C(D_800A1888[a0]);
    }
    func_8005E914();
}
