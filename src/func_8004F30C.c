extern int D_8009CF9C;
extern int D_8009CF98;
extern short D_800C0E48[];

extern unsigned char *func_80062A34(int, int);
extern int func_80063428(unsigned char *);
extern void func_80057D30(int);
extern void func_80062F1C(int);
extern void func_80062F3C(int);
extern unsigned char *func_80062A20(int, int);
extern void func_8004E704(int);
extern void func_800525EC();
extern void func_80063198(unsigned char *);
extern void func_80052634();

int func_8004F30C(int a0, int a1) {
    unsigned char *p;
    int v;

    if ((a1 & 0x10000) != 0) {
        p = func_80062A34(2, 1);
        D_8009CF9C = D_800C0E48[func_80063428(p)];
        v = func_80063428(p) | (*(int *)(p + 0x5C) << 8);
        D_8009CF98 = v + 1;
        func_80057D30(v & 0xFF);
        func_80062F1C(a0);
        func_80062F3C(0x1B);
        func_80062F3C(1);
        func_80062F3C(0);
        func_8004E704(func_80063428(func_80062A20(a0, 0)));
        func_800525EC();
    } else if ((a1 & 0x40) != 0) {
        func_80062F1C(a0);
        func_80063198(func_80062A34(1, 0));
        func_80063198(func_80062A34(1, 1));
        func_80063198(func_80062A34(1, 0x1B));
        func_80052634();
    }
    return 1;
}
