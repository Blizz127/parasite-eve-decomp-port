/* VRAM 0x8004542C / file 0x35C2C / size 0xF0. */
extern int D_8009CF1C;
extern int D_8009CF18;

extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80064C20(unsigned char *);
extern void func_80064B74(unsigned char *, int);
extern void func_80062CB8(unsigned char *);
extern void func_80045A98();
extern void func_80045D0C();
extern void func_8004F978();
extern void func_8004FFD0();

void func_8004542C(int a0) {
    unsigned char *p;
    unsigned char *q;
    int flag;

    p = func_80062D2C(5, a0, 0, 0);
    q = func_8006322C(5, p, p);
    flag = D_8009CF1C;
    *(unsigned int *)(p + 0x30) = (unsigned int)func_80045A98;
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_80045D0C;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004F978;
    if (flag != 0) {
        func_80064C20(q);
    } else if (D_8009CF18 == 0) {
        func_80064B74(q, 0x12);
    }
    if (*(int *)(q + 0x44) >= 0) {
        func_80062CB8(q);
    }
    *(int *)(p + 0x40) = 1;
    q = func_8006322C(0x1B, p, p);
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004FFD0;
    func_80064C20(q);
}
