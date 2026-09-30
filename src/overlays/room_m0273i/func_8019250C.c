/* room_m0273i — func_8019250C, blob offset 0x3524, 0xA4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * Room init (func_80192594 twin shape): $3-pinned last copy temp so return 0 fills the load delay slot. */

extern char D_8019A8D8[];
extern char D_8019A8B8[];
extern int D_8019A8CC, D_8019A8D0, D_8019A8D4;
extern void func_80192638();
extern void func_80079754();
extern void func_800785D4();

int func_8019250C(unsigned char *a0)
{
    *(void **)(a0 + 0xC) = func_80192638;
    a0[0x1A] = 0;
    a0[0x4A] = 0;
    *(short *)(a0 + 0x48) = 0;
    a0[0x4B] = 0;
    a0[3] = 0xFF;
    func_80079754(D_8019A8D8, 0x1F800028);
    func_800785D4(D_8019A8B8, 0x1F800028, a0 + 0x1C);
    *(int *)(a0 + 0x30) = D_8019A8CC;
    *(int *)(a0 + 0x34) = D_8019A8D0;
    {
        register int t asm("$3") = D_8019A8D4;
        *(int *)(a0 + 0x38) = t;
    }
    return 0;
}
