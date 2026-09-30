extern unsigned char *D_8009CEF4;
extern int D_800C0E44;

extern unsigned char *func_800424B4(int, int);
extern int func_80063428(unsigned char *);
extern void func_800622B0(int);
extern void func_8005E8A4(int, int);
extern void func_800614AC(int);
extern void func_80061A3C(int, int, int);
extern int func_8005BCB0(void);
extern void func_8005F27C(unsigned char *);
extern void func_8005EB64(int);
extern void func_8005FA3C(int);
extern void func_8005F5B8(int);
extern void func_800605F8(int);
extern void func_8005FB74(int);
extern void func_8005FDF0(int);
extern void func_8006006C(int, int);
extern unsigned char *func_8005DD8C(int);
extern void func_80064C54(int);

void func_800434C0(int a) {
    unsigned char *p;
    unsigned char *q;
    unsigned char *x;

    p = func_800424B4(*(int *)(D_8009CEF4 + 0x24) - 0x25, a);
    if (p == 0) {
        return;
    }
    switch (p[0]) {
    case 1:
        if (a == func_80063428(D_8009CEF4)) {
            func_800622B0(*(int *)(p + 0x20));
        } else {
            func_8005E8A4(-2, -2);
            func_800614AC(*(int *)(p + 0x20));
            q = D_8009CEF4;
            func_80061A3C(*(int *)(q + 0x3C), *(int *)(q + 0x40), 1);
            func_800614AC(D_800C0E44);
            func_8005E8A4(2, 2);
        }
        func_8005E8A4(2, 2);
        if (func_8005BCB0() != 0) {
            x = p + 0x14;
        } else {
            x = p + 4;
        }
        func_8005F27C(x);
        func_8005E8A4(0x58, 0);
        if (p[0x2A] != 0) {
            func_8005EB64(0x95);
            func_8005E8A4(0x1A, 2);
            func_8005EB64(0x96);
            func_8005E8A4(0x22, 1);
            func_8005FA3C(p[0x2A] + 1);
            func_8005E8A4(4, -3);
        } else {
            func_8005E8A4(0x4A, 0);
        }
        func_8005F5B8(0x5A);
        func_8005E8A4(0x19, 0);
        func_800605F8(a + 1);
        func_8005E8A4(-0xC4, 0xF);
        func_8005EB64(0x53);
        func_8005E8A4(0x1E, 1);
        func_8005FB74(p[0x28] + 1);
        func_8005E8A4(8, -1);
        func_8005EB64(0x54);
        func_8005E8A4(0x10, 1);
        func_8005FDF0(*(short *)(p + 0x24));
        func_8005EB64(0x4C);
        func_8005E8A4(5, 0);
        func_8005FDF0(*(short *)(p + 0x26));
        func_8005E8A4(8, -1);
        func_8005EB64(0x55);
        func_8005E8A4(0x1A, 1);
        func_8006006C(*(int *)(p + 0xC), 0);
        func_8005E8A4(-0xC0, 0xC);
        if (p[0x29] != 0) {
            func_8005F5B8(0x60);
        } else {
            func_8005F5B8(0xB);
            func_8005E8A4(0x1E, 0);
            func_800605F8(*(short *)(p + 0x2C));
            func_8005E8A4(0x14, 0);
            func_8005F27C(func_8005DD8C(*(short *)(p + 0x2E)));
        }
        break;
    case 2:
        func_8005E8A4(0, 0x12);
        func_80064C54(0x41);
        func_8005E8A4(0x5C, -0x10);
        break;
    case 3:
        func_8005E8A4(0, 0x12);
        func_80064C54(0x76);
        func_8005E8A4(0x5C, -0x10);
        break;
    }
}
