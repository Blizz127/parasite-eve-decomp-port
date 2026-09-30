extern unsigned char D_800BCC34[];
extern unsigned int D_800BCD50;
extern unsigned char D_800B8AC0[];
extern unsigned int D_8009D2C4;

extern void func_8008A400(int a0, int a1);
extern void func_8008A750(unsigned char *a0, unsigned char *s, unsigned int m, int a3);
extern void func_8008A8CC(unsigned char *a0, unsigned int a1);
extern void func_80089960(void);
extern void func_80089B28(void);
extern void func_80089CF0(void);

void func_8008A92C(unsigned char *a0, int a1, int a2)
{
    register unsigned char *rec asm("$22");
    register int p1 asm("$20");
    register int p2 asm("$21");
    register int bits asm("$19");
    register unsigned char *cur asm("$18");
    register unsigned int bit asm("$17");
    register int n asm("$16");
    int saved;
    int w;

    rec = a0;
    p1 = a1;
    p2 = a2;
    if (p1 == 0 && p2 == 0) {
        return;
    }
    w = *(int *)(rec + 8);
    if (w != 0) {
        func_8008A400(0, w);
    }
restart:
    cur = D_800BCC34;
    bits = D_800BCD50;
    bit = 0x800000;
    saved = bits;
    if (p1 != 0 && p2 != 0) {
        unsigned int pair;

        n = 11;
        cur -= 0x11C;
        bit = 0x400000;
        asm volatile("" : "=r"(bit) : "0"(bit));
        pair = bit << 1;
    pair_loop:
        if ((saved & (bit | pair)) == 0) {
            goto scan_done;
        }
        n -= 1;
        cur -= 0x11C;
        bit >>= 1;
        if (n == 0) {
            goto exhausted;
        }
        pair = bit << 1;
        goto pair_loop;
    } else {
        n = 12;
        asm volatile("" : "=r"(n) : "0"(n));
    single_loop:
        if ((saved & bit) == 0) {
            goto scan_done;
        }
        n -= 1;
        cur -= 0x11C;
        bit >>= 1;
        if (n != 0) {
            goto single_loop;
        }
    }
scan_done:
    if (n != 0) {
        goto work;
    }
exhausted:
    func_8008A400(0, 0x40000000);
    if (bits == (int)D_800BCD50) {
        bits = (int)0x80000000;
    }
    if (n != 0) {
        goto work;
    }
    if (bits >= 0) {
        goto restart;
    }
work:
    if (bits < 0) {
        return;
    }
    if (p1 != 0) {
        func_8008A750(cur, rec, bit, p1);
        func_8008A8CC(D_800B8AC0, *(unsigned int *)(cur + 0xF0));
    }
    if (p2 != 0) {
        if (p1 != 0) {
            cur += 0x11C;
            bit <<= 1;
        }
        func_8008A750(cur, rec, bit, p2);
        func_8008A8CC(D_800B8AC0, *(unsigned int *)(cur + 0xF0));
        if (p1 != 0) {
            *(unsigned int *)(cur + 0x38) |= 0x10000;
        }
    }
    D_8009D2C4 |= 0x10;
    func_80089960();
    func_80089B28();
    func_80089CF0();
}
