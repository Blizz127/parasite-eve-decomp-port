typedef struct Obj {
    unsigned char pad0[0x10];
    int f10;
    unsigned char pad14[0x88 - 0x14];
    int f88;
} Obj;

extern Obj **D_8009D1A8;
extern unsigned int D_8009D1AC;
extern unsigned char D_8009CE80;
extern char *D_800BCEAC;
extern unsigned char D_800BCEA8;
extern unsigned int D_800BCEB4;
extern char D_80091464[];
extern char D_80091474[];
extern char D_80091480[][22];
extern char D_800914AC[][20];
extern char D_800914D4[][20];
extern char D_800914FC[][18];
extern char D_80091520[][18];
extern char D_80091544[][22];
extern char D_80091570[][21];

extern void func_800374E8(void);
extern int func_8005BCB0(void);
extern void func_80037454(int, int, int, int);
extern void func_800375E0(int, int, short *);
extern void func_8005E894(int, int);
extern void func_80061C34(int, int, int, int);

void func_8003495C(void)
{
    short buf[3];
    unsigned int mode;
    unsigned int v;
    unsigned int f;

    if (D_8009D1A8 == 0 || *D_8009D1A8 == 0 || (*D_8009D1A8)->f10 <= 0) {
        func_800374E8();
        D_8009D1AC &= ~0x300;
    }
    mode = (D_8009D1AC >> 8) & 3;
    switch (mode) {
    case 1:
        buf[0] = (*D_8009D1A8)->f10;
        buf[1] = (*D_8009D1A8)->f88;
        buf[2] = 0;
        func_800374E8();
        func_80037454(func_8005BCB0() ? 0x14 : 0x61, D_8009CE80 < 2 ? 0xF : 0xC3, 0, 0);
        func_800375E0(0, 2, buf);
        v = (D_8009D1AC >> 10) & 3;
        if (v != 0) {
            if (v == mode) {
                D_800BCEAC = D_80091474;
            }
        } else {
            D_800BCEAC = D_80091464;
        }
        D_800BCEA8 = 2;
        {
            register unsigned int t asm("$4") = D_8009D1AC & ~0x300;
            D_8009D1AC = t | ((((D_8009D1AC >> 8) & 3) + 1) & 3) << 8;
        }
        D_800BCEB4 |= 0x2000000;
        break;
    case 2:
        if (*(unsigned char *)&D_8009D1AC == 0) {
            D_8009D1AC = (D_8009D1AC & ~0x300) | 0x200;
            *(unsigned char *)&D_8009D1AC = 0x4B;
            f = D_8009D1AC;
            if (f & 0x1000) {
                D_800BCEAC = D_80091480[func_8005BCB0()];
                D_8009D1AC &= ~0x1000;
            } else if (f & 0x2000) {
                D_800BCEAC = D_800914AC[func_8005BCB0()];
                D_8009D1AC &= ~0x2000;
            } else if (f & 0x4000) {
                D_800BCEAC = D_800914D4[func_8005BCB0()];
                D_8009D1AC &= ~0x4000;
            } else if (f & 0x8000) {
                D_800BCEAC = D_800914FC[func_8005BCB0()];
                D_8009D1AC &= ~0x8000;
            } else if (f & 0x10000) {
                D_800BCEAC = D_80091520[func_8005BCB0()];
                D_8009D1AC &= ~0x10000;
            } else if (f & 0x20000) {
                D_800BCEAC = D_80091544[func_8005BCB0()];
                D_8009D1AC &= ~0x20000;
            } else if (f & 0x40000) {
                D_800BCEAC = D_80091570[func_8005BCB0()];
                D_8009D1AC &= ~0x40000;
            } else {
                func_800374E8();
                D_8009D1AC &= ~0x300;
            }
        }
        break;
    }
    (*(unsigned char *)&D_8009D1AC)--;
    func_8005E894(0, D_8009CE80 < 2 ? 0xB : 0xBF);
    func_80061C34(0x140, 0x14, 0, 0);
}
