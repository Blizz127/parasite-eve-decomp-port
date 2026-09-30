extern unsigned char D_800922F4[];

extern unsigned char *func_80062D2C(int, int, int, int);
extern void func_80062CB8(unsigned char *);
extern void func_8004B970();
extern void func_8004BB80();

void func_8004B90C(void) {
    unsigned char *p;

    p = func_80062D2C(0x14, 0, 0, 0);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_8004B970;
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004BB80;
    func_80062CB8(p);
    *(unsigned int *)(p + 0x4C) = (unsigned int)D_800922F4;
}
