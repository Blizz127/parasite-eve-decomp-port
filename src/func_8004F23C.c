extern int func_80062CC4();
extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern int func_80062A34(int, int);
extern void func_800631AC(int);
extern void func_8004F30C();
extern void func_8004F2E4();

void func_8004F23C(void) {
    unsigned char *p;
    unsigned char *q;

    p = func_80062D2C(0x40, func_80062CC4(), 0, 0);
    q = func_8006322C(0x40, p, p);
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004F30C;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004F2E4;
    func_80062CB8(q);
    func_800631AC(func_80062A34(1, 0));
    func_800631AC(func_80062A34(1, 1));
    func_800631AC(func_80062A34(1, 0x1B));
}
