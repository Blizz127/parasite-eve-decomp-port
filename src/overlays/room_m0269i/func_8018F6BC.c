/* room_m0269i — func_8018F6BC, blob offset 0x6D4, 0x13C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * Room exit/teleport commit: clears player flags, func_8003E0FC warp, position fixup, BGM flag; separate p/q/u locals per D_8009D254 reload. */

extern char *D_8009D254;
extern unsigned int D_800BCF88;
extern void func_80020CE4();
extern void func_8003E0FC();
extern void func_8001AA78();

int func_8018F6BC(unsigned char *a0)
{
    char *p;
    char *q;
    char *u;

    if (*(int **)(a0 + 0x10) != 0) {
        **(int **)(a0 + 0x10) = 0;
    }
    a0[0] = 4;
    if (a0[0x28] != 0) {
        if (*(short *)(*(char **)D_8009D254 + 0xC) > 0) {
            func_80020CE4();
        }
        p = D_8009D254;
        *(int *)(p + 0x1D8) = 0;
        *(short *)(p + 0x1DC) = 0;
        *(int *)(p + 0x98) &= ~0x10000;
        *(unsigned short *)(p + 0x250) &= ~0x400;
        func_8003E0FC(*(int *)(a0 + 0x1C) + 0x1B4, 6, p + 0x28);
        q = D_8009D254;
        *(int *)(q + 0x28) <<= 16;
        *(int *)(q + 0x30) <<= 16;
        func_8001AA78();
        u = D_8009D254;
        *(int *)(u + 0x68) = 0;
        *(int *)(u + 0x6C) = 0;
        *(int *)(u + 0x70) = 0;
        *(int *)(u + 0x78) = 0;
        *(int *)(u + 0x7C) = 0;
        *(int *)(u + 0x80) = 0;
        *(int *)(u + 0x40) = *(int *)(u + 0x28);
        *(int *)(u + 0x44) = *(int *)(u + 0x2C);
        *(int *)(u + 0x48) = *(int *)(u + 0x30);
        D_800BCF88 |= 0x80;
        a0[0x28] = 0;
    }
    return 0;
}
