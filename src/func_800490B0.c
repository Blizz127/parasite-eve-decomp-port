extern unsigned short D_800A18D8[];
extern short D_800C0E28[];
extern int D_800C0E10;
extern int D_8009CF68;
extern int D_8009CF0C;
extern int D_8009CF34;
extern int D_8009CF30;
extern int D_8009CEF8;

extern void func_8005218C();
extern void func_80062F3C(int);
extern unsigned char *func_80062A34(int, int);
extern void func_80062CB8(unsigned char *);
extern unsigned char *func_80062CC4();
extern void func_80063198(int);
extern void func_800439D8();

void func_800490B0(void) {
    short *d;
    int i;
    unsigned short *s;

    d = D_800C0E28;
    i = 0;
    s = D_800A18D8;
    do {
        *d = *s;
        s += 2;
        i++;
        d++;
    } while (i < 7);
    D_800C0E10 = D_8009CF68;
    func_8005218C();
    func_80062F3C(0xF);
    func_80062F3C(0xB);
    func_80062F3C(0xD);
    func_80062F3C(0x18);
    func_80062F3C(0x30);
    if (D_8009CF0C != 0) {
        func_80062CB8(func_80062A34(2, 0x32));
    } else {
        if (func_80062CC4() != 0) {
            func_80063198(*(int *)(func_80062CC4() + 4));
        }
        func_80063198((int)func_80062A34(1, 0x1B));
    }
    D_8009CF34 = 0;
    D_8009CF30 = 0;
    if (D_8009CF0C == 0) {
        func_80062CB8(func_80062A34(2, 0));
        func_800439D8();
        D_8009CEF8 = 1;
    }
}
