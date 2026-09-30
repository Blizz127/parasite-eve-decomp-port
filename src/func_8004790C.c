extern int D_8009CF24;
extern int D_8009CF28;
extern int D_8009CF18;
extern int D_8009CF2C;

extern int func_80059F08(int);
extern void func_8005E8A4(int, int);
extern void func_8005E8C4();
extern unsigned char *func_8005332C(int);
extern void func_8005EB64(int);
extern void func_8005F5B8(int);
extern int func_8005DC4C(int);
extern int func_8005F1A0(int);
extern void func_8005E914();
extern void func_800536B8(int);

void func_8004790C(void) {
    int h;
    register int x asm("$4");
    unsigned char *p;
    int t;

    h = func_80059F08(D_8009CF24);
    func_8005E8A4(4, 4);
    func_8005E8C4();
    if (D_8009CF28 >= 0) {
        p = func_8005332C(h);
        t = (p + D_8009CF28)[0x15] & 0x1F;
        if (D_8009CF18 != 0) {
            func_8005EB64(t + 0x22);
        } else {
            func_8005EB64(t + 0x36);
        }
        x = 0x24;
    } else {
        func_8005F5B8(0x66);
        x = func_8005F1A0(func_8005DC4C(0x66));
    }
    func_8005E8A4(x, 0);
    func_8005F5B8((D_8009CF2C & 1) | 0x64);
    func_8005E914();
    if ((D_8009CF2C & 1) == 0) {
        func_8005E8A4(0, 0xE);
        func_800536B8(h);
        func_8005E8A4(0, 0xE);
        func_8005F5B8(0x67);
    }
    func_8005E8A4(0, 0x14);
    func_8005F5B8(0x68);
}
