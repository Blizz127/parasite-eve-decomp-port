/* VRAM 0x8004B03C / file 0x3B83C / size 0x68.
 * Save/load file-menu page 0x23: same skeleton as func_8004AF3C with its
 * own callbacks func_8004B0A4 / func_8004FF80.  era -O2 -G0. */
extern void func_8004B0A4();
extern void func_8004FF80();
extern int func_80062D2C();
extern int func_8006322C();
extern int func_80062CB8();

int func_8004B03C(int arg0) {
    unsigned char *s0 = (unsigned char *)func_80062D2C(0x23, arg0, 0, 0);
    unsigned char *a0 = (unsigned char *)func_8006322C(0x23, s0, s0);
    *(int *)(s0 + 0x2C) = (int)func_8004B0A4;
    *(int *)(a0 + 0x30) = (int)func_8004FF80;
    return func_80062CB8(a0);
}
