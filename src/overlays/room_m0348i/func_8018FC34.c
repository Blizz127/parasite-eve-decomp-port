/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_8018FC34 — blob offset 0xc4c, 0x58 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80190D4C; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
void func_8018FC34(int a0, void *q, void *p)
{
    H(p, 0x10) += 200;
    H(p, 0x12) -= 16;
    H(p, 0x14) += 1;
    if (H(p, 0x14) >= 8) {
        B(q, 0x1) = 2;
    }
    if (H(p, 0x12) < 0) {
        H(p, 0x12) = 0;
    }
}
