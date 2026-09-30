/* room_m0107i (PE.IMG room m0107i chunk 2, VRAM 0x8018EFE8)
 * func_8018FCC8 — blob offset 0xce0, 0x64 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0421i func_8018FB44; C re-targeted by symbol address
 * (docs/evidence/room_m0107i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_8018FCC8(int a0, void *q, void *p)
{
    H(p, 0x10) += 15;
    if (H(q, 0x2) < 16) {
        H(p, 0x12) += 8;
    } else {
        H(p, 0x12) -= 4;
    }
    H(p, 0xA) += 100;
    if (H(p, 0x12) <= 0) {
        B(q, 0x1) = 2;
    }
}
