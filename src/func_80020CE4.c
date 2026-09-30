extern unsigned int D_8009D2E8;
extern unsigned int *D_8009D254;
extern unsigned int *D_8009D278;
extern void func_8001A680(void *a0, int a1);

void func_80020CE4(void) {
    unsigned int *t;
    unsigned int *s;

    D_8009D2E8 &= 0xFFFFFFFE;
    t = D_8009D278;
    t[0x13] &= 0xFFFEFFFF;
    s = D_8009D254;
    s[0x1A] = 0;
    s[0x1B] = 0;
    s[0x1C] = 0;
    func_8001A680(s, ((unsigned char *)t)[0x12]);
}
