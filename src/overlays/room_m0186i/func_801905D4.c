/* room_m0186i — func_801905D4, blob offset 0x15EC, 0x1A0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * 8-particle init; pointer-arith x store keeps the D_800942EC load after it (in-struct store lets sched hoist it); zero stores after the vy store. */

typedef struct { short x, y, z, pad; } V4;
extern short D_800942EC;
extern unsigned char *func_800C2B50();
extern int func_80071A54();

void func_801905D4(int a0, unsigned char *a1, unsigned char *a2)
{
    unsigned char *r;
    unsigned int i;
    V4 *v = (V4 *)a2;

    r = func_800C2B50();
    for (i = 0; i < 8; i++) {
        *(short *)&v[i] = *(int *)(r + 0x18);
        v[i].y = D_800942EC;
        v[i].z = *(int *)(r + 0x20);
        v[i + 8].x = func_80071A54() % 8192 - 0x1000;
        v[i + 8].z = func_80071A54() % 8192 - 0x1000;
        v[i + 8].y = -(func_80071A54() % 8000 + 8000);
        v[i + 16].x = 0;
        v[i + 16].y = 0;
        v[i + 16].z = func_80071A54() % 4096;
        (a2 + i)[0xC0] = 1;
        (a2 + i)[0xC8] = func_80071A54() % 3;
    }
    *(short *)(a2 + 0xD2) = 0x80;
    *(short *)(a2 + 0xD0) = 0x200;
    *(short *)(a2 + 0xD6) = 0xC8;
    a2[0xD4] = 8;
}
