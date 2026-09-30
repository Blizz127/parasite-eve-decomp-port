/* room_m0418i — func_801943CC, blob offset 0x53E4, 0xCC bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * entity init from func_800C2B50 pos (<<16) + func_800C2B10/2B28 params; lever: zero stores after the +8 copy */

extern unsigned char *func_800C2B50();
extern int *func_800C2B10();
extern int *func_800C2B28();

void func_801943CC(void *a0, void *a1, unsigned char *a2)
{
    unsigned char *e = func_800C2B50();

    *(int *)(a2 + 0) = *(int *)(e + 0x18) << 16;
    *(int *)(a2 + 4) = *(int *)(e + 0x1C) << 16;
    *(int *)(a2 + 8) = *(int *)(e + 0x20) << 16;
    *(short *)(a2 + 0x20) = 0;
    *(short *)(a2 + 0x22) = 0;
    *(short *)(a2 + 0x24) = 0;
    *(int *)(a2 + 0x2C) = *func_800C2B10(2);
    a2[0x28] = *func_800C2B10(3);
    *(short *)(a2 + 0x2A) = *func_800C2B28(1);
    if (a2[0x28] == 0) {
        *(int *)(a2 + 0x18) = 0x124F80;
    } else {
        *(int *)(a2 + 0x18) = -0x124F80;
    }
    *(int *)(a2 + 0x10) = 0x7A120;
    *(int *)(a2 + 0x14) = -0xAAE60;
}
