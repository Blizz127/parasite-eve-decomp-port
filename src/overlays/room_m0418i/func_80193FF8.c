/* room_m0418i — func_80193FF8, blob offset 0x5010, 0x98 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * entity init from func_800C2B50 record (+0x18/+0x1C/+0x20 position offsets) */

extern int *func_800C2B10();
extern unsigned char *func_800C2B50();

void func_80193FF8(void *a0, void *a1, unsigned char *a2)
{
    unsigned char *e = func_800C2B50();

    func_800C2B10(1);
    *(short *)(a2 + 0x8C) = 0x14;
    *(short *)(a2 + 0x88) = 0x80;
    *(short *)(a2 + 0x8A) = 0;
    *(short *)(a2 + 0x30) = 0;
    *(short *)(a2 + 0x32) = 0x400;
    *(short *)(a2 + 0x34) = 0;
    *(int *)(a2 + 0) = *(int *)(e + 0x18) - 0x15E;
    *(int *)(a2 + 4) = *(int *)(e + 0x1C) + 0x78;
    {
        int z = *(int *)(e + 0x20);
        *(short *)(a2 + 0x48) = 0;
        *(short *)(a2 + 0x4A) = 0x800;
        *(short *)(a2 + 0x4C) = 0;
        *(int *)(a2 + 8) = z;
    }
}
