extern int D_8009CF30;

extern unsigned char *func_80062A20(unsigned char *, int);
extern unsigned char *func_80062A34(int, int);
extern void func_80063D30(unsigned char *);
extern void func_80062CB8(unsigned char *);
extern void func_80063158(unsigned char *, int, int);
extern void func_80063198(unsigned char *);
extern void func_8005267C(void);
extern int func_80063428(unsigned char *);
extern void func_8004A570(unsigned char *, int);
extern void func_800525EC(void);
extern void func_800490B0(void);
extern void func_80052634(void);

int func_800491C8(unsigned char *a, int f0) {
    register unsigned char *p asm("$16");
    unsigned char *q;
    register int v asm("$2");
    register int flags asm("$17");
    register int t asm("$5");

    flags = f0;


    p = func_80062A20(a, 0);
    if (flags & 0x4000) {
        *(int *)(p + 0x44) = -1;
        p = func_80062A34(2, 0x10);
        *(int *)(p + 0x44) = 0;
        *(int *)(p + 0x48) = 0;
        func_80063D30(p);
        func_80062CB8(p);
        p = func_80062A34(1, 0xF);
        q = func_80062A20(func_80062A34(1, 0xD), 0);
        v = D_8009CF30;
        t = *(int *)(p + 0x18);
        if (v == 0) {
            v = *(int *)(q + 0x80);
            if (v == 0) {
                v = 0x9C;
            } else {
                v = 0xA2;
            }
        } else {
            v = 0xB0;
        }
        flags = v - t;
        func_80063158(p, flags, 0);
        func_80063198(p);
        p = func_80062A34(1, 0xB);
        func_80063158(p, flags, 0);
        func_80063198(p);
        p = func_80062A34(1, 0x2F);
        func_80063158(p, flags, 0);
        func_80063198(p);
        func_8005267C();
    } else if (flags & 0x10000) {
        func_8004A570(p, func_80063428(p) + 5);
        func_800525EC();
    } else {
        v = flags & 0x40;
        if (v != 0) {
            func_800490B0();
            func_80052634();
        }
    }
    return 1;
}
