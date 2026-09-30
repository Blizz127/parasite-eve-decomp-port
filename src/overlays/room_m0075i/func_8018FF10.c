/* room_m0075i (PE.IMG room m0075i chunk 2, VRAM 0x8018EFE8)
 * func_8018FF10 — blob offset 0xF28, 0x64 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Effect step keyed on q+2: before 16 grow +0x10 and shrink +0x12; done at 16. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_8018FF10(int a0, void *q, void *p)
{
    if (H(q, 0x2) < 16) {
        H(p, 0x10) += 240;
    }
    if (H(q, 0x2) < 16) {
        H(p, 0x12) -= 8;
    }
    if (H(q, 0x2) == 16) {
        B(q, 0x1) = 2;
    }
}
