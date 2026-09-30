extern int func_80059F08(int);
extern unsigned char *func_8005332C(int);
extern void func_8005E8A4(int, int);
extern void func_800536B8(int);
extern void func_8004551C(unsigned char *);
extern void func_8005FDF0(int);
extern void func_8005FF28(int);
extern void func_8005EB64(int);

void func_80047C50(void) {
    int h;
    unsigned char *p;

    h = func_80059F08(1);
    p = func_8005332C(h);
    func_8005E8A4(4, 4);
    func_800536B8(h);
    func_8005E8A4(-4, -4);
    func_8004551C(p);
    if (p != 0) {
        func_8005E8A4(0x2A, -0xC);
        func_8005FDF0(p[9]);
        func_8005E8A4(5, 0);
        func_8005FF28(*(short *)(p + 0x12));
        func_8005E8A4(-0x2D, -0xE);
        func_8005FDF0(p[8]);
        func_8005E8A4(5, 0);
        func_8005FF28(*(short *)(p + 0x10));
        func_8005E8A4(-0x2D, -0xE);
        func_8005FDF0(p[7]);
        func_8005E8A4(5, 0);
        func_8005FF28(*(short *)(p + 0xE));
        func_8005E8A4(-0x2D, -0xA);
        func_8005EB64(0x87);
        func_8005E8A4(0x19, 0);
        func_8005EB64(0x88);
    }
}
