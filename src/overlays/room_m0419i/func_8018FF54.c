/* room_m0419i (PE.IMG room m0419i chunk 2, VRAM 0x8018EFE8)
 * func_8018FF54 — blob offset 0xf6c, 0x64 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0075i func_8018FF10; C re-targeted by symbol address
 * (docs/evidence/room_m0419i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_8018FF54(int a0, void *q, void *p)
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
