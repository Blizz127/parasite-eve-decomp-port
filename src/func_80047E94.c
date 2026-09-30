extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80052E30(int);
extern int func_80059F08(int);
extern unsigned char *func_8005332C(int);
extern void func_800647D0(unsigned char *, int);
extern void func_80064C20(unsigned char *);
extern unsigned char *func_80062A34(int, int);
extern void func_80059C44();
extern void func_80047F48();
extern void func_8004FAF8();

void func_80047E94(int a0) {
    unsigned char *p;
    unsigned char *q;
    unsigned char *r;

    p = func_80062D2C(0xB, a0, 0, 0);
    q = func_8006322C(0xB, p, p);
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_80047F48;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004FAF8;
    func_80052E30(0);
    func_800647D0(q, func_8005332C(func_80059F08(1))[0x14]);
    func_80064C20(q);
    r = func_80062A34(2, 6);
    if (r != 0) {
        *(unsigned int *)(q + 0x78) = (unsigned int)r;
        *(unsigned int *)(r + 0x7C) = (unsigned int)q;
    }
    func_80059C44();
}
