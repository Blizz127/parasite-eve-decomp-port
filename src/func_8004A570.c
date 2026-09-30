typedef struct {
    char b[0x20];
} Blk;

extern Blk D_800A1A00;
extern int D_800A18D8[];
extern int D_8009CF68;
extern int D_8009CFD0;
extern int D_8009CFD8;
extern int D_8009CFDC;
extern int D_8009CFAC;

extern unsigned char *func_80062D2C(int, int, int, int);
extern void func_80062CB8(unsigned char *);
extern int func_80059F08(int);
extern Blk *func_8005332C(int);
extern void func_80063158(unsigned char *, int, int);
extern void func_8004A6CC();
extern void func_8004A9A0();

void func_8004A570(int a0, int a1) {
    unsigned char *p;

    p = func_80062D2C(9, a0, 0, 1);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_8004A6CC;
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004A9A0;
    *(int *)(p + 0x28) = 1;
    func_80062CB8(p);
    D_8009CFD0 = a1;
    D_8009CFD8 = D_8009CF68;
    if (a1 < 3) {
        D_800A1A00 = *func_8005332C(func_80059F08(0));
        *(int *)(p + 0x34) = *(int *)(p + 0x34) - 0x14;
        func_80063158(p, 0xA, 0);
    } else {
        D_8009CFDC = D_800A18D8[a1];
    }
    D_8009CFAC = 0;
}
