/* room_m0075i (PE.IMG room m0075i chunk 2, VRAM 0x8018EFE8)
 * func_80190AF4 — blob offset 0x1B0C, 0x60 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Effect draw setup via func_800C2EAC/2FF0/3098/3238, then func_800C5A40 on +0xAC and +0x78. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_800C5A40();
void func_80190AF4(int a0, int a1, void *p)
{
    func_800C2EAC(B(func_800C2B50(), 0x6C));
    func_800C2FF0(16, 64);
    func_800C3098(16);
    func_800C3238(2);
    func_800C5A40((char *)p + 0xAC);
    func_800C5A40((char *)p + 0x78);
}
