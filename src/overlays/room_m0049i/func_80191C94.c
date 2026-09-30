/* room_m0049i — func_80191C94, blob offset 0x2CAC, 0xB0 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl 2026-09-27).
 * entity init via func_800C2B10 slots; +0x10 = rand()%40 */

extern int *func_800C2B10();
extern int func_80071A54();

void func_80191C94(void *a0, void *a1, unsigned char *a2)
{
    *(short *)(a2 + 0x2E) = 0xD;
    *(short *)(a2 + 0x34) = *func_800C2B10(1);
    *(short *)(a2 + 0x24) = *func_800C2B10(2);
    *(short *)(a2 + 0x26) = 1;
    *(short *)(a2 + 0x2A) = 0x80;
    *(short *)(a2 + 0x28) = *func_800C2B10(1);
    *(short *)(a2 + 0x2C) = 0;
    *(int *)(a2 + 0x30) = 0;
    *(short *)(a2 + 0x22) = 0;
    *(short *)(a2 + 0x14) = 0;
    *(short *)(a2 + 0x10) = func_80071A54() % 40;
}
