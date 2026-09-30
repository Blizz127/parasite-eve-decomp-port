extern unsigned int D_800B0CD8;
extern unsigned char *D_8009D254;
extern unsigned int D_8009D1A0;
extern unsigned int D_8009D2E8;
extern void *D_800B0D10;
extern short D_800B0D14;
extern short D_800B0D16;
extern unsigned char D_800B89F8;
extern void func_800661A4(void);
extern void func_8003A088(void *a0);
extern void func_8003AC90(void *a0, void *a1);
extern void func_8003AF14(void *a0, void *a1);
extern void func_800661CC(void);

int func_8006CC68(void)
{
    register unsigned char *g asm("$16");
    register unsigned char *s1 asm("$17");
    unsigned char *o;

    g = (unsigned char *)&D_800B0CD8;
    if ((*(unsigned int *)g & 0xC0000) != 0) {
        return 0;
    }
    o = D_8009D254;
    if (o == 0) {
        return 0;
    }
    if ((*(unsigned int *)(o + 0x98) & 0x20000040) != 0) {
        return 0;
    }
    if ((D_8009D1A0 & 2) == 0) {
        if ((D_8009D2E8 & 2) != 0) {
            return 0;
        }
    }
    if (D_800B0D10 == 0) {
        D_800B0D10 = o + 0x1B4;
        D_800B0D14 = 3;
        D_800B0D16 = 0x12;
    }
    g = g + 0x14;
    func_800661A4();
    func_8003A088(g);
    s1 = &D_800B89F8;
    func_8003AC90(g, s1);
    if ((D_8009D1A0 & 2) != 0 || D_8009D254[0x252] != 0) {
        func_8003AF14(g, s1);
    }
    func_800661CC();
    return 0;
}
