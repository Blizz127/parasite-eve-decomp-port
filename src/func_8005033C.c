extern int D_8009CF0C;
extern int D_8009CF34;

extern int func_80062A34(int, int);
extern int func_80063428(int);
extern int func_800556E8(int);
extern void func_80062F3C(int);
extern void func_80062CB8(int);
extern unsigned char *func_80062CC4();
extern void func_80063198(int);
extern void func_8004C5DC();
extern unsigned char *func_8005332C(int);
extern int func_800534CC(int);
extern void func_8004F490(int);

void func_8005033C(int a0, int a1) {
    int h;
    unsigned char *p;

    if (a1 != 0) {
        h = func_800556E8(func_80063428(func_80062A34(2, 0xD)));
        func_80062F3C(0xF);
        func_80062F3C(0xB);
        func_80062F3C(0xD);
        func_80062F3C(0x18);
        func_80062F3C(0x30);
        if (D_8009CF0C != 0) {
            func_80062CB8(func_80062A34(2, 0x32));
        } else {
            if (func_80062CC4() != 0) {
                func_80063198(*(int *)(func_80062CC4() + 4));
            }
            func_80063198(func_80062A34(1, 0x1B));
        }
        D_8009CF34 = 0;
        func_8004C5DC();
        p = func_8005332C(h);
        p[0x14] = p[0x14] + 1;
        func_8004F490(func_800534CC(h));
    }
}
