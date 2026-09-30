/* room_m0049i — func_801903A8, blob offset 0x13C0, 0x80 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * reads two func_800C2B10 slots into a2+0x2E/+0x24, seeds fields */

extern int *func_800C2B10();

void func_801903A8(void *a0, void *a1, unsigned char *a2)
{
    *(short *)(a2 + 0x2E) = *func_800C2B10(1);
    *(short *)(a2 + 0x24) = *func_800C2B10(2);
    *(short *)(a2 + 0x26) = 1;
    if (*(short *)(a2 + 0x24) == -1) {
        *(short *)(a2 + 0x2A) = 0;
        *(short *)(a2 + 0x28) = 0;
    } else {
        *(short *)(a2 + 0x2A) = 0x80;
        *(short *)(a2 + 0x28) = 0x38;
    }
    *(short *)(a2 + 0x2C) = 0;
    *(int *)(a2 + 0x30) = 0;
    *(short *)(a2 + 0x22) = 0;
}
