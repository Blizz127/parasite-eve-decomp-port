extern int D_8009CF0C;
extern int D_8009CFB8;

extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern int func_80062A34(int, int);
extern void func_800631AC(int);
extern void func_800471E4();
extern void func_800471BC();

void func_80046DFC(int a0, int a1) {
    unsigned char *p;
    unsigned char *q;

    p = func_80062D2C(a1 + 0x3A, a0, 0, 0);
    q = func_8006322C(a1 + 0x3A, p, p);
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_800471E4;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_800471BC;
    func_80062CB8(q);
    func_800631AC(func_80062A34(1, 0x33));
    if (a1 != 0) {
        D_8009CFB8 = D_8009CF0C;
    } else {
        D_8009CFB8 = 0;
    }
}
