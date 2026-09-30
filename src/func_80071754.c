extern void func_8001A784(void *a0, int a1);
extern int func_80079FB4(int a0, int a1);
extern unsigned short D_800BE9A0;
extern unsigned char D_800BE9A6;
extern unsigned char D_800BE9A7;
extern unsigned short D_800BD020;
extern unsigned short D_800BD022;
extern unsigned int D_8009D26C;
extern int **D_8009D254;
extern unsigned int D_8009D2E8;

void func_80071754(unsigned char *a0)
{
    register unsigned char *s asm("$16");
    int v3;
    int v2;
    unsigned int f;
    register int p0 asm("$4");
    register int p1 asm("$5");
    int v;
    int **o;

    s = a0;
    __asm__ __volatile__("" ::: "memory");
    func_8001A784(s, 0x11);
    if ((D_800BE9A0 & 0xF000) == 0x7000) {
        p0 = D_800BE9A7;
        p1 = D_800BE9A6;
        v3 = func_80079FB4(p0 - 0x80, p1 - 0x80) - 0x400;
        __asm__ __volatile__("" : "=r"(v3) : "0"(v3));
        if (v3 < 0) {
            v3 = v3 + 0x1000;
        }
        v2 = D_800BD022;
    } else {
        f = D_8009D26C;
        if ((f & 8) != 0) {
            if ((f & 0x40) != 0) {
                *(short *)(s + 0x3A) = 0x600;
                goto after;
            }
            if ((f & 0x10) != 0) {
                *(short *)(s + 0x3A) = 0xA00;
                goto after;
            }
            *(short *)(s + 0x3A) = 0x800;
            goto after;
        }
        if ((f & 0x20) != 0) {
            if ((f & 0x40) != 0) {
                *(short *)(s + 0x3A) = 0x200;
                goto after;
            }
            if ((f & 0x10) != 0) {
                *(short *)(s + 0x3A) = 0xE00;
                goto after;
            }
            *(short *)(s + 0x3A) = 0;
            goto after;
        }
        if ((f & 0x40) != 0) {
            *(short *)(s + 0x3A) = 0x400;
            goto after;
        }
        if ((f & 0x10) != 0) {
            *(short *)(s + 0x3A) = 0xC00;
        }
    after:
        v3 = *(short *)(s + 0x3A);
        v2 = D_800BD020;
    }
    o = D_8009D254;
    v3 = v3 + v2;
    if (o != 0) {
        if ((D_8009D2E8 & 0x10) != 0) {
            v3 = v3 + 0x800;
            v3 = v3 + ((((unsigned int *)(*o))[19] >> 7) & 0xC00);
        }
    }
    v3 = v3 & 0xFFF;
    *(short *)(s + 0x3A) = v3;
    __asm__ __volatile__("" ::: "memory");
    if ((D_8009D2E8 & 0x10) != 0) {
        v = 0x1000 - *(unsigned short *)(s + 0x3A);
        *(short *)(s + 0x3A) = v & 0xFFF;
    }
}
