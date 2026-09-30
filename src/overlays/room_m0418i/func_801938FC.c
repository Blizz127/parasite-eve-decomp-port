/* room_m0418i — func_801938FC, blob offset 0x4914, 0x1E8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl7 2026-09-27).
 * 10-slot jitter updater: active slots re-randomise SV8 D_801994D8[i] (rand()%160-80, %512-256), idle slots wake on rand()%20==0. */

typedef struct { short x, y, z, w; } SV8;
extern SV8 D_801994D8[];
extern int func_80071A54();

#define B(off) (a2 + i)[off]
#define H(off) *(short *)(a2 + i * 2 + (off))

void func_801938FC(int a0, unsigned char *a1, unsigned char *a2)
{
    unsigned int i;

    for (i = 0; i < 10; i++) {
        if (B(0x24) == 1) {
            H(0xC) += 1;
            D_801994D8[i].x = func_80071A54() % 160 - 80;
            D_801994D8[i].y = func_80071A54() % 160 - 80;
            D_801994D8[i].z = func_80071A54() % 160 - 80;
            D_801994D8[i].w = func_80071A54() % 512 - 256;
        } else if (func_80071A54() % 20 == 0) {
            B(0x24) = 1;
        }
    }
    if (*(short *)(a1 + 2) >= 0x3D) {
        a1[1] = 2;
    }
}
