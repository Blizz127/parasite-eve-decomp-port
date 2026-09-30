extern unsigned char *D_8009CF58;

extern void func_8004551C(unsigned char *);
extern void func_8005E8A4(int, int);
extern void func_8005FDF0(int);
extern void func_8005FF28(int);
extern void func_8005EB64(int);

void func_8004F644(void) {
    unsigned char *p;

    func_8004551C(D_8009CF58);
    p = D_8009CF58;
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
