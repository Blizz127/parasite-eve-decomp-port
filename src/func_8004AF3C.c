/* VRAM 0x8004AF3C / file 0x3B73C / size 0x68.
 * Save/load file-menu page 0x21: allocate the node through func_80062D2C,
 * bind it with func_8006322C, install the two page callbacks at +0x2C and
 * +0x30, then hand the bound node to func_80062CB8.  era -O2 -G0. */
extern void func_8004AFA4();
extern void func_8004FF58();
extern int func_80062D2C();
extern int func_8006322C();
extern int func_80062CB8();

int func_8004AF3C(int arg0) {
    unsigned char *s0 = (unsigned char *)func_80062D2C(0x21, arg0, 0, 0);
    unsigned char *a0 = (unsigned char *)func_8006322C(0x21, s0, s0);
    *(int *)(s0 + 0x2C) = (int)func_8004AFA4;
    *(int *)(a0 + 0x30) = (int)func_8004FF58;
    return func_80062CB8(a0);
}
