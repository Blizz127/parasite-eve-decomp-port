/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_80191DB0 — blob offset 0x2dc8, 0x6c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80192EC8; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_80191DB0(int a0, void *q, void *p)
{
    void *e = p;

    H(p, 0x30) -= 150;
    if (H(p, 0x30) < 400) {
        H(p, 0x30) = 400;
    }
    if (H(q, 0x2) >= 8) {
        H(e, 0x34) -= 16;
        if (H(e, 0x34) < 0) {
            H(e, 0x34) = 0;
            B(q, 0x1) = 2;
        }
    }
}
