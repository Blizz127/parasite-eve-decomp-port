extern void func_8005DE88();
extern int func_80062A34(int, int);
extern int func_80054294();
extern void func_80048918(int, int, int);
extern void func_8005C488();
extern void func_8004DD64(int);
extern unsigned char *func_80062D2C(int, int, int, int);
extern void func_8004C608();

void func_8004DCA4(int a0) {
    unsigned char *p;

    func_8005DE88();
    if (func_80062A34(1, 0xD) != 0) {
        return;
    }
    if (func_80062A34(1, 0x17) != 0) {
        return;
    }
    if (a0 != 0) {
        if (func_80054294() != 0) {
            func_80048918(0, -2, -1);
        } else {
            func_8005C488();
            return;
        }
    } else {
        func_8004DD64(-1);
    }
    if (func_80062A34(1, 0x13) != 0) {
        return;
    }
    p = func_80062D2C(0x13, 0, 0, 0);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_8004C608;
}
