extern int D_8009CFB8;
extern int D_8009CF94;
extern int D_8009CF8C;
extern int D_8009CF98;
extern int D_8009CF00;

extern void func_80062F3C(int);
extern void func_800439D8();
extern unsigned char *func_80062CC4();
extern unsigned char *func_80062D2C(int, unsigned char *, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern void func_80055760();
extern int func_80052F70();
extern void func_800647D0(unsigned char *, int);
extern unsigned char *func_80062A34(int, int);
extern void func_80063198(unsigned char *);
extern void func_800447F0();
extern void func_80044444();
extern void func_8004F8D0();
extern void func_80057C54();
extern void func_80050260();

void func_80046EAC(int a0) {
    unsigned char *c;
    unsigned char *r;
    unsigned char *p;
    unsigned char *q;
    int v;
    int b1;
    register int b2 asm("$3");

    func_80062F3C(0x3C);
    func_80062F3C(0x3B);
    func_80062F3C(0x3A);
    switch (D_8009CFB8) {
    case 0:
        if (a0 != 0) {
            c = func_80062CC4();
            *(int *)(c + 0x48) = 0;
            r = func_80062D2C(0x1B, 0, 0, 0);
            *(unsigned int *)(r + 0x30) = (unsigned int)func_800447F0;
            p = func_80062D2C(1, c, 0, 0);
            q = func_8006322C(1, p, p);
            *(unsigned int *)(p + 0x2C) = (unsigned int)func_80044444;
            *(unsigned int *)(q + 0x30) = (unsigned int)func_8004F8D0;
            func_80062CB8(q);
            *(unsigned int *)(q + 0x84) = (unsigned int)func_80057C54;
            *(unsigned int *)(q + 0x88) = (unsigned int)func_80050260;
            func_80055760();
            D_8009CF94 = -1;
            D_8009CF8C = -1;
            func_800647D0(q, func_80052F70());
            v = D_8009CF98;
            D_8009CF00 = 0;
            if (v != 0) {
                v = v - 1;
                *(int *)(q + 0x44) = v & 1;
                b1 = (v >> 1) & 0x7F;
                b2 = v >> 8;
                *(int *)(q + 0x48) = b1;
                *(int *)(q + 0x5C) = b2;
            }
        } else {
            func_800439D8();
        }
        break;
    case 1:
        func_80063198(func_80062A34(1, 0x33));
        break;
    case 2:
        func_80063198(func_80062A34(1, 0x33));
        break;
    }
}
