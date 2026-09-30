extern int D_8009D004;
extern int D_800B0CD8[];
extern unsigned char D_80092354[];
extern unsigned char D_80092380[];
extern unsigned char D_800923A0[];

extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern int func_8005BCB0();
extern unsigned char *func_80062A34(int, int);
extern void func_80062CB8(unsigned char *);
extern int func_8005332C(int);
extern void func_8005BCBC(int);
extern void func_8005BE1C();
extern void func_80052FCC(int);
extern void func_8004E2E4();
extern void func_80050178();
extern void func_800501C8();
extern void func_8005010C();
extern void func_8004E074();
extern void func_800500A8();
extern void func_8004DF74();

void func_8004DD64(int a0) {
    unsigned char *s;
    unsigned char *e;
    unsigned char *tbl;
    int *f;
    int v;

    s = func_80062D2C(0x17, 0, 0, 0);
    *(unsigned int *)(s + 0x2C) = (unsigned int)func_8004E2E4;
    v = func_8005BCB0();
    tbl = D_80092354;
    if (v != 0) {
        tbl = D_80092380;
    }
    *(unsigned int *)(s + 0x4C) = (unsigned int)tbl;
    if (func_8005BCB0() != 0) {
        e = func_8006322C(0x18, s, s);
        *(unsigned int *)(e + 0x30) = (unsigned int)func_80050178;
        *(int *)(e + 0x1C) = *(int *)(e + 0x1C) - 0x34;
        e = func_8006322C(0x19, s, s);
        *(unsigned int *)(e + 0x30) = (unsigned int)func_800501C8;
        *(int *)(e + 0x1C) = *(int *)(e + 0x1C) - 0x34;
    } else {
        *(unsigned int *)(func_8006322C(0x17, s, s) + 0x30) = (unsigned int)func_8005010C;
        *(unsigned int *)(func_8006322C(0x18, s, s) + 0x30) = (unsigned int)func_80050178;
        e = func_8006322C(0x19, s, s);
        *(unsigned int *)(e + 0x30) = (unsigned int)func_800501C8;
    }
    *(int *)(e + 0x44) = -1;
    s = func_80062D2C(0x11, (int)func_80062A34(2, 0x17), 0, 0);
    e = func_8006322C(0x11, s, s);
    *(unsigned int *)(s + 0x2C) = (unsigned int)func_8004E074;
    *(unsigned int *)(e + 0x30) = (unsigned int)func_800500A8;
    *(int *)(e + 0x44) = 0;
    *(int *)(e + 0x48) = 0;
    func_80062CB8(e);
    s = func_80062D2C(0x1A, 0, 0, 0);
    *(unsigned int *)(s + 0x30) = (unsigned int)func_8004DF74;
    D_8009D004 = func_8005332C(a0);
    if (D_8009D004 == 0) {
        *(unsigned int *)(s + 0x4C) = (unsigned int)D_800923A0;
    }
    func_8005BCBC(D_8009D004);
    func_8005BE1C();
    func_80052FCC(D_8009D004);
    f = D_800B0CD8;
    *f = *f | 0x8000;
}
