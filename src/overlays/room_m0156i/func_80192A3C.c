/* room_m0156i — func_80192A3C, blob offset 0x3A54, 0xA0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * camera/window param block D_80192C34 set + refresh from D_800F32D0->8; lever: returns &D_80192C34 (keeps $v0 live) */

typedef struct { short a, b, c; short pad; int d; } S34;
extern S34 D_80192C34;
extern unsigned char *D_800F32D0;
extern unsigned short D_800942EC;

S34 *func_80192A3C(int a0, int a1, int a2, int a3)
{
    unsigned char *p;

    if (a0 == 1) {
        D_80192C34.a = a1;
        D_80192C34.b = a2;
        D_80192C34.c = a3;
    } else {
        D_80192C34.d = a1;
    }
    p = D_800F32D0;
    D_80192C34.a = *(unsigned short *)(*(unsigned char **)(p + 8) + 0x268);
    D_80192C34.b = *(unsigned short *)(*(unsigned char **)(p + 8) + 0x26A);
    D_80192C34.c = *(unsigned short *)(*(unsigned char **)(p + 8) + 0x26C);
    D_80192C34.b = D_800942EC;
    D_80192C34.d = 0x76C;
    return &D_80192C34;
}
