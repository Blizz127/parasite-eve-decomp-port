extern unsigned char D_800A1A20[];
extern int func_80062A34(int, int);
extern int func_80062CC4();
extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern void func_8004D024(int);
extern unsigned char *func_8005DC4C(int);
extern void func_80052BCC(unsigned char *, unsigned char *);
extern void func_80052C08(unsigned char *, unsigned char *);
extern int func_8005F1A0(unsigned char *);
extern void func_8005DE88();
extern void func_8004CDD4();
extern void func_8004D030();
extern void func_8004FFA8();

void func_8004CC50(int a0, int a1)
{
    unsigned char *p;
    unsigned char *q;
    int has;
    int id;
    int w;

    has = func_80062A34(1, 0x28) != 0;
    id = has ? 0x3D : 0x28;
    p = func_80062D2C(id, func_80062CC4(), 0, 1);
    q = func_8006322C(id, p, p);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_8004CDD4;
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004D030;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004FFA8;
    func_80062CB8(q);
    func_8004D024(0);
    func_80052BCC(D_800A1A20 + (has << 6), func_8005DC4C(a0));
    if (a1 != 0) {
        func_80052C08(D_800A1A20 + (has << 6), func_8005DC4C(a1));
    }
    w = (func_8005F1A0(D_800A1A20 + (has << 6)) < 100) ? 100 : func_8005F1A0(D_800A1A20 + (has << 6));
    *(int *)(p + 0x34) = w + 0x14;
    *(int *)(p + 0x18) = (0x12C - w) >> 1;
    *(int *)(q + 0x18) = *(int *)(p + 0x34) - 0x44;
    func_8005DE88();
}
