/* room_m0424i (PE.IMG room m0424i chunk 2, VRAM 0x8018EFE8)
 * func_8018F91C — blob offset 0x934, 0x20 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0107i func_8018FAE4; C re-targeted by symbol address
 * (docs/evidence/room_m0424i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_8018F91C(int a0, int a1, void *p)
{
    H(p, 0x10) = 256;
    H(p, 0x8) = 0;
    H(p, 0xA) = 0;
    H(p, 0xC) = 0;
    H(p, 0x12) = 1;
}
