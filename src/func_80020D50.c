extern unsigned int *D_8009D278;
extern unsigned int *D_8009D254;
extern unsigned char D_8009CE30;
extern void func_8001A680(void *a0, int a1);
extern void func_80021D4C(void);

void func_80020D50(void) {
    unsigned int *s;
    unsigned int *t;

    if (((short *)D_8009D278)[6] > 0) {
        s = D_8009D254;
        s[0x1A] = 0;
        s[0x1B] = 0;
        s[0x1C] = 0;
        func_8001A680(s, 0x12);
        t = D_8009D278;
        t[0x13] |= 0x2000;
        D_8009CE30 = 0;
        s = D_8009D254;
        s[0x26] &= 0xFFFFFEFF;
        func_80021D4C();
    }
}
