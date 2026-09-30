extern int D_8009CF34;
extern int D_8009CF38;
extern int *D_8009CEF4;
extern int D_8009CF1C;
extern int D_8009CF30;
extern int D_800A1888;
extern int D_800A188C;
extern int D_800A1890;
extern int D_800A1894;

extern int func_80054288();
extern int func_80055FE0(int);
extern void func_8005EB58(int);
extern int func_800556E8(int);
extern int func_80054240(int);
extern unsigned char *func_8005332C(int);
extern int func_8005415C(int);
extern int func_80054520(int);
extern void func_800536B8(int);
extern void func_80064C80();

void func_800430A0(int a0) {
    int h;
    int s2;
    int ok;
    int m;

    if (a0 < func_80054288()) {
        if (D_8009CF34 != 0 || D_8009CF38 != 0) {
            if (func_80055FE0(a0) == 0) {
                func_8005EB58(1);
            }
        }
        h = func_800556E8(a0);
        s2 = func_80054240(h);
        if (s2 != 0) {
            if (D_8009CEF4[9] != 0xD) {
                if (D_8009CF1C == 0 && D_8009CF30 == 0) {
                    func_8005EB58(1);
                }
            }
        }
        if (D_8009CF1C != 0) {
            ok = 0;
            if (D_800A1888 + D_800A188C + D_800A1890 + D_800A1894 != 0) {
                if ((func_8005332C(h)[5] & 0x40) == 0) {
                    if (func_8005415C(h) < 6) {
                        m = 0x3E;
                    } else {
                        m = 1 << func_8005415C(h);
                    }
                    if (func_80054520(m) != 0) {
                        ok = 1;
                    }
                }
            }
            if (ok == 0) {
                func_8005EB58(1);
            }
        }
        func_800536B8(h);
        if (s2 != 0) {
            func_80064C80();
        }
    }
}
