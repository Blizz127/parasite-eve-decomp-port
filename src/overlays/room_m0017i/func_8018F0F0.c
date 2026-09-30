/* room_m0017i — func_8018F0F0, blob offset 0x108, 0xB4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * one-shot func_8018F1C4 setup then per-frame model update chain on +8 record +0x1B4 */

extern int D_800BCFA4;
extern void func_8018F1C4();
extern void func_8018F1F8();
extern void func_8003A088();
extern void func_8003AC90();
extern void func_8018F580();

int func_8018F0F0(unsigned char *a0)
{
    if (a0[0x18] == 0) {
        func_8018F1C4(*(unsigned char **)(a0 + 8) + 0x1B4, *(int *)(a0 + 0xC),
                      *(short *)(a0 + 0x10), *(short *)(a0 + 0x12),
                      *(short *)(a0 + 0x14), *(short *)(a0 + 0x16));
        a0[0x18] = 1;
    }
    if (a0[0x19] == 0) {
        return 0;
    }
    func_8018F1F8(*(unsigned char **)(a0 + 8) + 0x1B4);
    func_8003A088(*(unsigned char **)(a0 + 8) + 0x1B4);
    func_8003AC90(*(unsigned char **)(a0 + 8) + 0x1B4, D_800BCFA4);
    func_8018F580(*(unsigned char **)(a0 + 8) + 0x1B4);
    return 0;
}
