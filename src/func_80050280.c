extern short D_800922D0[];

extern void func_8005E8A4(int, int);
extern void func_8005EB64(int);

void func_80050280(int a0) {
    short *q;

    func_8005E8A4(0, 2);
    func_8005EB64(D_800922D0[a0]);
    func_8005E8A4(0x12, 4);
    func_8005EB64(0x22);
    func_8005E8A4(0xC, -4);
    q = D_800922D0;
    if (a0 == 0) {
        q = q + 1;
    }
    func_8005EB64(*q);
}
