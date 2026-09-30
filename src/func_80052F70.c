extern unsigned char D_800C0E0C;
extern int func_80051E58(void);

int func_80052F70(void) {
    int x = func_80051E58();
    unsigned char *s0 = &D_800C0E0C;

    if (s0[0] + x >= 0x33) return 0x32;
    return s0[0] + func_80051E58();
}
