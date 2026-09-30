extern int D_8009CF80;
extern int D_8009CF40;
extern int D_8009CF60;
extern int D_8009CF64;
extern int D_8009CF68;
extern int D_8009CF6C;
extern int D_8009CF70;
extern int D_8009CF74;
extern int D_800A18D8[];
extern int D_800A18FC[];
extern short D_800C0E28[];
extern unsigned char D_800C0E0A;
extern short D_800C0E06;
extern int D_800C0E10;

extern int func_80055668();
extern void func_8005218C();
extern void func_80062F1C(int);
extern unsigned char *func_80062A34(int, int);
extern void func_80062F3C(int);
extern void func_80052764();
extern int func_80057ECC();
extern void func_80048654();
extern void func_800512AC(int, int);
extern void func_800525EC();

int func_8004C1E0(int a0, int a1) {
    short *d;
    int i;
    int *pb;
    int *pa;
    register int n asm("$4");

    if ((a1 & 0x10000) != 0) {
        if (D_8009CF80 != 0) {
            D_8009CF40 = 0x80;
            D_8009CF64 = D_8009CF70;
            D_8009CF68 = D_8009CF74;
            while (D_8009CF60 < D_8009CF6C) {
                n = D_8009CF60 + 1;
                D_8009CF60 = n;
                if (func_80055668() != 0) {
                    break;
                }
            }
        } else {
            d = D_800C0E28;
            i = 0;
            pb = D_800A18FC;
            pa = D_800A18D8;
            do {
                *d = *pa + *pb;
                pb++;
                pa++;
                i++;
                d++;
            } while (i < 9);
            D_800C0E0A = D_8009CF60;
            D_800C0E06 = D_8009CF64;
            D_800C0E10 = D_8009CF68;
            func_8005218C();
            func_80062F1C(a0);
            func_80062F1C((int)func_80062A34(1, 0x1E));
            func_80062F3C(0x1C);
            func_80052764();
            if (func_80057ECC() != 0) {
                func_80048654();
            } else {
                func_800512AC(0xA, 0);
            }
        }
        func_800525EC();
    }
    return 1;
}
