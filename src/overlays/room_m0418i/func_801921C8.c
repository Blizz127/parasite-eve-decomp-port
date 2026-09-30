/* room_m0418i — func_801921C8, blob offset 0x31E0, 0x210 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * 4-slot rising-spark updater (timers, rsin/rcos offsets, respawn on W<0 with rand()); H/W/V per-array macros. */

extern int func_80071A54();
extern int func_80077CF4(), func_80077DC4();

#define H(off) *(short *)(a2 + i * 2 + (off))
#define W(off) *(int *)(a2 + i * 4 + (off))
#define V(off) *(short *)(a2 + i * 8 + (off))

void func_801921C8(int a0, unsigned char *a1, unsigned char *a2)
{
    unsigned int i;

    *(short *)(a2 + 0x5A) += 1;
    if (*(short *)(a2 + 0x58) < 0x1000) {
        *(short *)(a2 + 0x58) += 0xC8;
    }
    for (i = 0; i < 4; i++) {
        H(0x50) = (short)(H(0x50) + 1) % 40;
        W(0x28) -= 0xC8;
        V(8) = func_80077CF4(*(short *)(a1 + 2) << 3) >> 3;
        V(0xA) = func_80077DC4(*(short *)(a1 + 2) << 4) >> 3;
        V(8) = 0;
        V(0xA) = 0;
        V(0xC) += H(0x48);
        if (H(0x38) < 0x80) {
            H(0x38) += 8;
        }
        if (W(0x28) < 0) {
            H(0x50) = func_80071A54() % 10;
            W(0x28) = 0x2000;
            H(0x38) = 0;
            V(0xC) = func_80071A54() % 4096;
            H(0x48) = func_80071A54() % 10 + 0x14;
        }
    }
    if (*(short *)(a2 + 0x5A) >= 0xB5) {
        a1[1] = 2;
    }
}
