/* room_m0418i — func_80192CCC, blob offset 0x3CE4, 0xA4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * fade in/out on a2+2 by phase a2+4; lever: if/else stores cross-jumped (reload of +4) */

void func_80192CCC(void *a0, unsigned char *a1, unsigned char *a2)
{
    short v;

    if (*(short *)(a1 + 2) < 0x3D) {
        *(short *)(a2 + 4) = 1;
    } else {
        *(short *)(a2 + 4) = 2;
    }
    if (*(short *)(a2 + 4) == 1) {
        *(short *)(a2 + 2) = v = *(short *)(a2 + 2) + 8;
        if (v >= 0x80) {
            *(short *)(a2 + 2) = 0x7F;
        }
    }
    if (*(short *)(a2 + 4) == 2) {
        *(short *)(a2 + 2) = v = *(short *)(a2 + 2) - 8;
        if (v < 0) {
            *(short *)(a2 + 2) = 0;
        }
    }
    if (*(short *)(a1 + 2) >= 0x79) {
        a1[1] = 2;
    }
}
