/* room_m0418i — func_8018F8B0, blob offset 0x8C8, 0xA8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * per-frame table ramps (+0x254/+0x25C x4, +0x230 x2), clamp +0x264 */

void func_8018F8B0(void *a0, unsigned char *a1, unsigned char *a2)
{
    unsigned int i;
    short t;

    for (i = 0; i < 4; i++) {
        ((short *)(a2 + 0x254))[i] += 0x3C;
        ((short *)(a2 + 0x25C))[i] += 0x80;
    }
    for (i = 0; i < 2; i++) {
        if (((short *)(a2 + 0x230))[i] < 0) {
            ((short *)(a2 + 0x230))[i] = 0;
        }
        ((short *)(a2 + 0x230))[i] -= 0x1E;
    }
    t = *(short *)(a2 + 0x264);
    if (t >= 3) {
        *(short *)(a2 + 0x264) = t - 2;
    }
    if (*(short *)(a1 + 2) == 0x28) {
        a1[1] = 2;
    }
}
