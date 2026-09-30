extern int D_8009CF80;
extern unsigned char D_80092314[];

extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern void func_8004BF08();
extern void func_8004BF40();
extern void func_8004C1E0();
extern void func_8004FFF8();

void func_8004BE4C(void) {
    unsigned char *p;
    unsigned char *q;

    p = func_80062D2C(0x15, 0, 0, 0);
    q = func_8006322C(0x2F, p, p);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_8004BF40;
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004C1E0;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004FFF8;
    *(int *)(q + 0x44) = -1;
    *(int *)(q + 0x18) += 0x44;
    *(int *)(q + 0x1C) += 2;
    *(int *)(q + 0x40) -= 2;
    func_80062CB8(p);
    *(unsigned int *)(p + 0x4C) = (unsigned int)D_80092314;
    D_8009CF80 = 1;
    func_8004BF08();
}
