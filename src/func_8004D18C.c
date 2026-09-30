extern int D_8009CF50;

extern int func_80062CC4();
extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern unsigned char *func_80062A34(int, int);
extern void func_8005C1EC(int);
extern void func_80042538();
extern void func_8005DE88();
extern void func_8004D2DC();
extern void func_8004FDE8();
extern void func_8004FDA4();
extern void func_8004C608();

void func_8004D18C(void) {
    unsigned char *p;
    unsigned char *q;
    unsigned char *r;
    int a0;

    p = func_80062D2C(0x24, func_80062CC4(), 0, 0);
    q = func_8006322C(0x24, p, p);
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004D2DC;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004FDE8;
    *(unsigned int *)(q + 0x8C) = (unsigned int)func_8004FDA4;
    a0 = 1;
    if (*(int *)(q + 0x48) == 2) {
        *(int *)(q + 0x48) = 0;
    }
    func_80062CB8(q);
    if (func_80062A34(1, 0x13) == 0) {
        r = func_80062D2C(0x13, 0, 0, 0);
        *(unsigned int *)(r + 0x30) = (unsigned int)func_8004C608;
    }
    *(int *)(func_80062A34(1, 0x13) + 0x38) = 0x24;
    D_8009CF50 = a0;
    func_8005C1EC(1);
    func_80042538();
    func_8005DE88();
}
