extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80047C50();
extern void func_80047D74();
extern void func_8004FFD0();

void func_80047BEC(int a0) {
    unsigned char *p;
    unsigned char *q;

    p = func_80062D2C(0xA, a0, 0, 0);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_80047C50;
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_80047D74;
    q = func_8006322C(0x1C, p, p);
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004FFD0;
    *(int *)(q + 0x44) = -1;
}
