/* room_m0418i — func_80191748, blob offset 0x2760, 0xE8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * timed trigger: func_800C6CE0==3 at frame 0xF0 sets player flag 0x4000 and model bit31 */

extern unsigned char **D_8009D254;
extern int *func_800C2B28();
extern int func_800C2B68();
extern int func_800C6CE0();
extern void func_80020D50();
extern void func_80020DD0();

void func_80191748(unsigned char *a0, unsigned char *a1)
{
    unsigned int *p;

    if (*func_800C2B28(1) == 1) {
        if (*(short *)(a1 + 2) < 0xF0) {
            func_80020D50();
        }
        if (*(short *)(a1 + 2) == 0xF0 && func_800C6CE0(a0) == 3) {
            *(int *)(*D_8009D254 + 0x4C) |= 0x4000;
            p = **(unsigned int ***)(a0 + 8);
            *p |= 0x80000000;
            func_80020DD0();
        }
    }
    if (func_800C2B68() == 1) {
        a1[1] = 2;
    }
}
