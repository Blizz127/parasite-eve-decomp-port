/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_8018F60C — blob offset 0x624, 0x70 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Store +2 through func_800C2B10(4), func_800C2B90(o, 2, ...), flag +1 = 2. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern int *func_800C2B10();
extern void func_800C2B90();
extern char D_80196CB8[];
extern char D_80196C7C[];
void func_8018F60C(void *o, void *q, void *p)
{
    *func_800C2B10(4) = H(p, 0x2);
    func_800C2B90(o, 2, D_80196CB8, D_80196C7C);
    B(q, 0x1) = 2;
}
