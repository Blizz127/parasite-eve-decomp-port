/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_8018F9AC — blob offset 0x9c4, 0x80 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80190AC4; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_8018F9AC(int a0, void *q, void *p)
{
    void *e = p;

    if (H(p, 0x14) == 1) {
        H(p, 0x10) += 200;
    }
    if (H(p, 0x14) == 0) {
        H(p, 0x10) -= 200;
    }
    H(e, 0x12) -= 16;
    if (H(e, 0x12) < 0) {
        H(e, 0x12) = 0;
    }
    if (H(q, 0x2) >= 21) {
        B(q, 0x1) = 2;
    }
}
