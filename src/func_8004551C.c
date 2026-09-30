extern int D_8009CF18;
extern int D_8009CF1C;

extern void func_8005E8A4(int, int);
extern void func_8005EB64(int);
extern unsigned char *func_80062CC4();
extern void func_80060590(int);

void func_8004551C(unsigned char *a0) {
    int base;
    int x;

    if (a0 != 0) {
        base = (D_8009CF18 != 0) ? 0x7C : 0x7F;
        func_8005E8A4(4, 0x1C);
        func_8005EB64(base);
        func_8005E8A4(0, 0xE);
        func_8005EB64(base + 1);
        func_8005E8A4(0, 0xE);
        func_8005EB64(base + 2);
        if (D_8009CF1C != 0 && *(int *)(func_80062CC4() + 0x24) == 7) {
            func_8005E8A4(0x18, 0xE);
        } else {
            func_8005E8A4(0x1E, -0x1C);
            x = a0[7] + *(short *)(a0 + 0xE);
            if (x >= 0x3E8) {
                x = 0x3E7;
            }
            func_80060590(x);
            func_8005E8A4(-0x24, 0xE);
            x = a0[8] + *(short *)(a0 + 0x10);
            if (x >= 0x3E8) {
                x = 0x3E7;
            }
            func_80060590(x);
            func_8005E8A4(-0x24, 0xE);
            x = a0[9] + *(short *)(a0 + 0x12);
            if (x >= 0x3E8) {
                x = 0x3E7;
            }
            func_80060590(x);
            func_8005E8A4(-0x24, 0xE);
        }
    }
}
