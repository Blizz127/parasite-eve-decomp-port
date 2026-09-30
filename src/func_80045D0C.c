/* VRAM 0x80045D0C / file 0x3650C / size 0x1D8. */
extern int D_8009CF1C;

extern unsigned char *func_80062A20(int, int);
extern unsigned char *func_80062A34(int, int);
extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern int func_80054288();
extern void func_80048254(void);
extern int func_80052558(void);
extern void func_80062CB8(unsigned char *);
extern void func_800525EC();
extern void func_800526C4();
extern void func_80047678(void);
extern void func_80062F3C(int);
extern void func_80062F1C(int);
extern void func_800439D8(void);
extern void func_80052634();
extern int func_800631C0(unsigned char *);
extern void func_8005267C(void);
extern void func_8004FA10();

int func_80045D0C(int a0, int pad) {
    unsigned char *p;
    unsigned char *q;

    p = func_80062A20(a0, 0);
    if (pad & 0x10000) {
        if (func_80054288() != 0) {
            if (D_8009CF1C != 0) {
                func_80048254();
                return 1;
            }
            if (func_80052558() != 0) {
                q = func_80062D2C(0x35, 0, 0, 0);
                q = func_8006322C(0x35, q, q);
                *(unsigned int *)(q + 0x30) = (unsigned int)func_8004FA10;
                p = func_80062A34(2, 7);
                *(int *)(p + 0x44) = 0;
                func_80062CB8(p);
                func_800525EC();
                return 1;
            }
        }
        func_800526C4();
        return 1;
    }
    if (pad & 0x40) {
        if (D_8009CF1C != 0) {
            func_80047678();
            return 1;
        }
        func_80062F3C(7);
        func_80062F1C(a0);
        a0 = (int)func_80062A34(1, 6);
        if (a0 != 0) {
            func_80062F1C(a0);
            func_800439D8();
            func_80052634();
        }
        return 1;
    }
    if (pad & 0x4000) {
        if (func_800631C0(func_80062A34(1, 6)) != 0) {
            *(int *)(p + 0x44) = -1;
            p = func_80062A20(a0, 1);
            if (p != 0) {
                *(int *)(p + 0x44) = -1;
            }
            p = func_80062A34(2, 6);
            if (p != 0) {
                *(int *)(p + 0x48) = 0;
                *(int *)(p + 0x44) = 0;
                func_80062CB8(p);
            }
            func_8005267C();
        }
    }
    return 1;
}
