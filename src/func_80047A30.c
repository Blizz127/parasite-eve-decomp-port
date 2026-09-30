extern int D_8009CF24;
extern int D_8009CF28;
extern int D_8009CF2C;
extern int D_800A1888[];
extern signed char D_800C0E20[];

extern unsigned char *func_80062A20(int, int);
extern int func_80063428(unsigned char *);
extern void func_80062F1C(int);
extern void func_8005A318(int, int, int, int);
extern void func_80062F3C(int);
extern unsigned char *func_80062A34(int, int);
extern void func_80062CB8(unsigned char *);
extern int func_80059F08(int);
extern void func_80048918(int, int, int);
extern int func_80052F0C();
extern void func_800512AC(int, int);
extern void func_800525EC();
extern void func_80052634();

int func_80047A30(int a0, int a1) {
    int k;
    int *p;
    int i;
    int h;
    signed char *s;
    unsigned char *q;

    if ((a1 & 0x10000) != 0) {
        k = func_80063428(func_80062A20(a0, 0));
        switch (k) {
        case 0:
            func_80062F1C(a0);
            func_8005A318(D_8009CF24, D_8009CF28, D_8009CF2C, D_800A1888[D_8009CF2C]);
            p = &D_800A1888[D_8009CF2C];
            *p = *p - (*p < 0x3E7);
            func_80062F3C(0x2C);
            func_80062F3C(0xB);
            func_80062F3C(0xA);
            func_80062F3C(5);
            func_80062F3C(6);
            func_80062F3C(7);
            q = func_80062A34(2, 0);
            if (q == 0) {
                q = func_80062A34(2, 0x32);
            }
            func_80062CB8(q);
            i = 0;
            func_80048918(0x33E, -1, func_80059F08(D_8009CF24 == 0));
            s = D_800C0E20;
            do {
                h = func_80059F08(i);
                if (func_80052F0C() == 0) {
                    if (h == s[0]) {
                        func_800512AC(2, 0);
                    } else if (h == s[2]) {
                        func_800512AC(3, 0);
                    }
                }
                i++;
            } while (i < 2);
            func_800525EC();
            break;
        case 1:
            func_80062F1C(a0);
            func_80052634();
            break;
        }
    } else if ((a1 & 0x40) != 0) {
        func_80062F1C(a0);
        func_80052634();
    }
    return 1;
}
