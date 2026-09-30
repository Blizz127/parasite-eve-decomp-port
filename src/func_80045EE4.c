extern int D_8009CF1C;
extern int D_8009CF18;

extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80064C20(unsigned char *);
extern void func_80064B74(unsigned char *, int);
extern void func_80062CB8(unsigned char *);
extern void func_8004620C();
extern void func_8004F9A0();

void func_80045EE4(int a0) {
    unsigned char *p;
    unsigned char *q;
    int t;
    int flag;

    p = func_80062D2C(6, a0, 0, 0);
    q = func_8006322C(6, p, p);
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004620C;
    *(int *)(p + 0x40) = 1;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004F9A0;
    t = *(int *)(q + 0x64);
    flag = D_8009CF1C;
    *(int *)(q + 0x64) = t | 0x80;
    if (flag != 0) {
        func_80064C20(q);
    } else if (D_8009CF18 == 0) {
        func_80064B74(q, 0x14);
    }
    if (*(int *)(q + 0x44) >= 0) {
        func_80062CB8(q);
    }
}
