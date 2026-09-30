extern int D_8009CF10;
extern int D_8009CF04;
extern int D_8009CF08;
extern int D_8009CF0C;
extern int D_8009CDA8;

extern void func_80052E30(int);
extern int func_8005415C(int);
extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern int func_80052558();
extern int func_800562A4(int);
extern int func_8005257C();
extern void func_80055760();
extern unsigned char *func_8005332C(int);
extern int func_80055FE0(int);
extern void func_80044B0C();
extern void func_8004F910();

void func_80044924(int a0, int a1, int a2) {
    unsigned char *p;
    register unsigned char *q asm("$17");
    unsigned char *r;
    int k;
    int kind;
    int flag;
    int ok;
    int d;

    D_8009CF10 = a1;
    func_80052E30(a1);
    D_8009CF04 = a2;
    k = func_8005415C(a2);
    if (k != 0xA) {
        if (k == 0xC || k == 0xD) {
            D_8009CDA8 = 2;
        } else if (k == 0xE || k == 0xF) {
            D_8009CDA8 = 2;
        } else if (k == 8 || k == 9) {
            D_8009CDA8 = 2;
        } else {
            D_8009CDA8 = 1;
        }
    } else {
        D_8009CDA8 = 0;
    }
    kind = 2;
    if (D_8009CDA8 == kind) {
        kind = 3;
    }
    p = func_80062D2C(kind, a0, 0, 1);
    q = func_8006322C(kind, p, p);
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_80044B0C;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004F910;
    func_80062CB8(q);
    flag = 0;
    if (D_8009CDA8 == 1) {
        if (func_80052558() == 0) {
            flag = 1;
        } else if (func_800562A4(D_8009CF04) == 0) {
            flag = 1;
        } else if (func_8005257C() != 0) {
            flag = 1;
        }
    }
    D_8009CF08 = flag;
    func_80055760();
    if (D_8009CDA8 == 0) {
        ok = 0;
        d = D_8009CF04;
        r = func_8005332C(d);
        if (func_80055FE0(d) != 0) {
            if (D_8009CF0C != 1) {
                ok = 1;
            } else if (r[6] != 0xA) {
                ok = 1;
            } else if (r[0xE] < 4) {
                ok = 1;
            }
        }
        if (ok == 0) {
            *(int *)(q + 0x48) = 1;
            return;
        }
    }
    if (D_8009CF08 != 0) {
        *(int *)(q + 0x48) = 1;
    }
}
