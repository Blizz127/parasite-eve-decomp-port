/* room_m0429i — func_80192B10, blob offset 0x3B28, 0xB0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * player state reset: clear motion vectors, copy pos to +0x40, set D_800BCF88 bit 7; lever: d pinned $4 (call arg) */

extern unsigned char *D_8009D254;
extern int D_800BCF88;
extern void func_8001AA78();

void func_80192B10(void *a0, unsigned char *a1)
{
    register unsigned char *d asm("$4") = D_8009D254;
    unsigned char *e;
    int *q;

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
    q = &D_800BCF88;
    *q |= 0x80;
    a1[0x38] = 0;
}
