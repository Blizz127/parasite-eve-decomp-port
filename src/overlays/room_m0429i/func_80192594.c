/* room_m0429i — func_80192594, blob offset 0x35AC, 0x9C bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * Room init: separate D_8019B310/14/18 word globals; the last copy through a $3-pinned temp so the return 0 fills the load delay slot (ovl park 9w -> 0). */

extern char D_8019B31C[];
extern char D_8019B2FC[];
extern int D_8019B310, D_8019B314, D_8019B318;
extern void func_80192688();
extern void func_80079754();
extern void func_800785D4();

int func_80192594(unsigned char *a0)
{
    *(void **)(a0 + 0xC) = func_80192688;
    a0[0x1A] = 0;
    a0[0x44] = 0;
    a0[3] = 0xFF;
    func_80079754(D_8019B31C, 0x1F800028);
    func_800785D4(D_8019B2FC, 0x1F800028, a0 + 0x1C);
    *(int *)(a0 + 0x30) = D_8019B310;
    *(int *)(a0 + 0x34) = D_8019B314;
    {
        register int t asm("$3") = D_8019B318;
        *(int *)(a0 + 0x38) = t;
    }
    return 0;
}
