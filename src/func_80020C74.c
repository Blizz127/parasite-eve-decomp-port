extern unsigned int D_8009D2E8;
extern unsigned int *D_8009D254;
extern unsigned int *D_8009D278;
extern short D_8009D298;
extern void func_8001A680(void *a0, int a1);
extern void func_80021D4C(void);

void func_80020C74(void) {
    register unsigned int *s asm("$4");
    register int t asm("$5");

    D_8009D2E8 |= 1;
    s = D_8009D254;
    s[0x26] &= 0xFFFFFEFF;
    t = (int)D_8009D278;
    *(unsigned int *)(t + 0x4C) |= 0x10000;
    D_8009D298 = 0;
    func_8001A680(s, 0x12);
    func_80021D4C();
}
