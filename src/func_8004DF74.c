extern unsigned char *D_8009D004;
extern unsigned char D_800C20A4[];

extern void func_8005E8A4(int, int);
extern void func_80053648(unsigned char *);
extern int func_8005F1A0(unsigned char *);
extern void func_8005EB64(int);
extern unsigned char *func_8005BEE8();
extern void func_8005F27C(unsigned char *);
extern int func_8005BF08();
extern void func_80061A3C(int, int, int);

void func_8004DF74(void) {
    char pad[8];
    unsigned char *t;
    register int x asm("$4");
    int n;
    int i;

    if (D_8009D004 != 0) {
        func_8005E8A4(0x1C, 0xC);
        func_80053648(D_8009D004);
        t = D_800C20A4;
        if (D_8009D004[6] == 9) {
            t = t + 0x10;
        }
        x = func_8005F1A0(t) + 0x14;
    } else {
        func_8005E8A4(2, 2);
        func_8005EB64(0x47);
        func_8005E8A4(0x3C, 8);
        func_8005F27C(func_8005BEE8());
        x = func_8005F1A0(func_8005BEE8());
    }
    func_8005E8A4(x, 0xC);
    n = func_8005BF08();
    if (n != 0) {
        i = n - 1;
        do {
            func_80061A3C(9, 4, 0);
            func_8005E8A4(0xB, 0);
            i--;
        } while (i != -1);
    }
}
