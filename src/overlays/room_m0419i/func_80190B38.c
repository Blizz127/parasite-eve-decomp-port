/* room_m0419i (PE.IMG room m0419i chunk 2, VRAM 0x8018EFE8)
 * func_80190B38 — blob offset 0x1b50, 0x60 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0075i func_80190AF4; C re-targeted by symbol address
 * (docs/evidence/room_m0419i-ports-2026-09-23/REPORT.md). */

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
void func_80190B38(int a0, int a1, void *p)
{
    func_800C2EAC(B(func_800C2B50(), 0x6C));
    func_800C2FF0(16, 64);
    func_800C3098(16);
    func_800C3238(2);
    func_800C5A40((char *)p + 0xAC);
    func_800C5A40((char *)p + 0x78);
}
