extern int D_800B0CD8;
extern unsigned int D_8009D1A0;
extern void func_80074F44(short *a0, int a1, int a2, int a3);
extern void func_80074DC0(int a0);
extern void func_80087024(void);
extern void func_8003DFC8(int a0);
extern void func_800696F0(void);

void func_8003F2FC(void) {
    int *s;
    int x;

    s = &D_800B0CD8;
    __asm__ __volatile__("" : "=r"(s) : "0"(s));
    if (*s & 0x200) {
        short buf[4];

        buf[0] = 0;
        buf[1] = 0;
        buf[2] = 0x140;
        buf[3] = 0x1C0;
        func_80074F44(buf, 0, 0, 1);
    }
    func_80074DC0(0);
    func_80087024();
    func_8003DFC8(1);
    func_800696F0();
    x = *s | 2;
    D_8009D1A0 = (D_8009D1A0 | 0x40) & ~0x3800;
    *s = x & ~0x800;
    if (x & 0x200) {
        *s = (*s | 2) & ~0x8200;
    }
}
