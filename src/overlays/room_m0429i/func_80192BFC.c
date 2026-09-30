/* room_m0429i — func_80192BFC, blob offset 0x3C14, 0x108 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * teardown + player reset (clears *a0->0x10 first) */

extern unsigned char *D_8009D254;
extern int D_800BCF88;
extern void func_80020CE4();
extern void func_8001AA78();

int func_80192BFC(unsigned char *a0)
{
    unsigned char *s = a0 + 0xC;
    register unsigned char *d asm("$4");
    unsigned char *e;
    unsigned char *f;

    **(int **)(a0 + 0x10) = 0;
    a0[0] = 4;
    if (a0[0x44] != 0) {
        f = *(unsigned char **)D_8009D254;
        if (f != 0 && *(short *)(f + 0xC) > 0) {
            func_80020CE4();
        }
        d = D_8009D254;
        *(int *)(d + 0x1D8) = 0;
        *(short *)(d + 0x1DC) = 0;
        *(int *)(d + 0x98) &= ~0x10000;
        *(unsigned short *)(d + 0x250) &= ~0x400;
        func_8001AA78(d);
        e = D_8009D254;
        *(int *)(e + 0x68) = 0;
        *(int *)(e + 0x6C) = 0;
        *(int *)(e + 0x70) = 0;
        *(int *)(e + 0x78) = 0;
        *(int *)(e + 0x7C) = 0;
        *(int *)(e + 0x80) = 0;
        *(int *)(e + 0x40) = *(int *)(e + 0x28);
        *(int *)(e + 0x44) = *(int *)(e + 0x2C);
        *(int *)(e + 0x48) = *(int *)(e + 0x30);
        D_800BCF88 |= 0x80;
        s[0x38] = 0;
    }
    return 0;
}
