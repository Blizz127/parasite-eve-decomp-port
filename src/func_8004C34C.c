extern int D_8009CF94;
extern int D_8009CF8C;
extern int D_8009CF98;
extern int D_8009CF00;
extern int D_8009CF5C;
extern int D_8009CEFC;

extern unsigned char *func_80062A34(int, int);
extern void func_8005DE88();
extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern void func_80055760();
extern int func_80052F70();
extern void func_800647D0(unsigned char *, int);
extern void func_8004C594();
extern void func_80055E14();
extern void func_8005B890(int);
extern void func_800447F0();
extern void func_80044444();
extern void func_8004F8D0();
extern void func_80057C54();
extern void func_80050260();

int func_8004C34C(int a0) {
    unsigned char *p;
    unsigned char *q;
    unsigned char *r;
    int flag;
    int v;
    int b1;
    register int b2 asm("$3");

    flag = (func_80062A34(1, 1) != 0);
    if (flag == 0) {
        func_8005DE88();
        r = func_80062D2C(0x1B, 0, 0, 0);
        *(unsigned int *)(r + 0x30) = (unsigned int)func_800447F0;
        p = func_80062D2C(1, 0, 0, 0);
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
        func_8004C594();
        *(int *)(func_80062A34(2, 1) + 0x84) = 0;
        func_80055E14();
        D_8009CF00 = 1;
        D_8009CF5C = a0;
        D_8009CEFC = 0;
        func_8005B890(0);
    }
    return flag;
}
