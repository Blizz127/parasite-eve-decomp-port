extern int D_8009CF44;

extern unsigned char *func_80062A34(int, int);
extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_800647D0(unsigned char *, int);
extern int func_80063428(unsigned char *);
extern int func_800631DC();
extern void func_80062CB8(unsigned char *);
extern void func_8005DE88();
extern void func_8004D6D4();
extern void func_8004FEEC();
extern void func_8004FE58();
extern void func_8004D690();

int func_8004D4C4(int a0, int a1) {
    unsigned char *p;
    unsigned char *q;
    unsigned char *r;
    int h;

    p = func_80062D2C(a0 + 0x25, (int)func_80062A34(2, 0x24), 0, 0);
    q = func_8006322C(a0 + 0x25, p, p);
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004D6D4;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004FEEC;
    *(unsigned int *)(q + 0x8C) = (unsigned int)func_8004FE58;
    func_800647D0(q, a1);
    h = func_80063428(q);
    if (func_800631DC() == 0) {
        *(int *)(q + 0x44) = 0;
        func_80062CB8(q);
    } else {
        *(int *)(q + 0x44) = -1;
    }
    r = func_80062D2C(0x3F, (int)q, 0, 0);
    *(unsigned int *)(r + 0x30) = (unsigned int)func_8004D690;
    func_8005DE88();
    D_8009CF44 = a0;
    return h;
}
