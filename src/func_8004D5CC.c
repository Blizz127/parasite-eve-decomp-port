extern void (*D_8009CFFC)();
extern int D_8009CF44;

extern unsigned char *func_80062A34(int, int);
extern void func_80062F3C(int);
extern void func_80062F1C(unsigned char *);
extern int func_800631DC();
extern void func_80062CB8(unsigned char *);

void func_8004D5CC(int a0) {
    unsigned char *p;

    p = func_80062A34(2, 0x24);
    func_80062F3C(0x28);
    if (D_8009CFFC != 0) {
        D_8009CFFC();
        D_8009CFFC = 0;
    }
    if (a0 == D_8009CF44) {
        func_80062F3C(0x3F);
        func_80062F3C(0x27);
        func_80062F1C(func_80062A34(1, 0x29));
        func_80062F3C(0x1F);
        func_80062F3C(a0 + 0x25);
        if (p != 0) {
            *(int *)(p + 0x44) = 0;
            if (func_800631DC() == 0) {
                func_80062CB8(p);
            }
        }
    }
}
