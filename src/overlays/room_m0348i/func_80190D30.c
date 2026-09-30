/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_80190D30 — blob offset 0x1d48, 0x40 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80191E48; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_80190D30(int a0, void *q, void *p)
{
    H(p, 0x12) -= 8;
    H(p, 0x10) += 30;
    H(p, 0x2) -= 15;
    if (H(p, 0x12) < 0) {
        H(p, 0x12) = 0;
        B(q, 0x1) = 2;
    }
}
