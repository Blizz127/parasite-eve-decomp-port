/* room_m0269i — func_8018F5D4, blob offset 0x5EC, 0xE8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * player reset variant (func_8003E0FC pose, pos<<16, D_800BCF88|=0x80); levers: $6/$4 pins for the call-argument bases */

extern unsigned char *D_8009D254;
extern int D_800BCF88;
extern void func_8003E0FC();
extern void func_8001AA78();

void func_8018F5D4(void *a0, unsigned char *a1)
{
    register unsigned char *d asm("$6") = D_8009D254;
    register unsigned char *e asm("$4");
    unsigned char *f;
    int *q;

    *(int *)(d + 0x1D8) = 0;
    *(short *)(d + 0x1DC) = 0;
    *(int *)(d + 0x98) &= ~0x10000;
    *(unsigned short *)(d + 0x250) &= ~0x400;
    func_8003E0FC(*(unsigned char **)(a1 + 0x10) + 0x1B4, 6, d + 0x28);
    e = D_8009D254;
    *(int *)(e + 0x28) <<= 16;
    *(int *)(e + 0x30) <<= 16;
    func_8001AA78(e);
    f = D_8009D254;
    *(int *)(f + 0x68) = 0;
    *(int *)(f + 0x6C) = 0;
    *(int *)(f + 0x70) = 0;
    *(int *)(f + 0x78) = 0;
    *(int *)(f + 0x7C) = 0;
    *(int *)(f + 0x80) = 0;
    *(int *)(f + 0x40) = *(int *)(f + 0x28);
    *(int *)(f + 0x44) = *(int *)(f + 0x2C);
    *(int *)(f + 0x48) = *(int *)(f + 0x30);
    q = &D_800BCF88;
    *q |= 0x80;
    a1[0x1C] = 0;
}
