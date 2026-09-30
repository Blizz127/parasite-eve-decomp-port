/* room_m0107i (PE.IMG room m0107i chunk 2, VRAM 0x8018EFE8)
 * func_8018FAE4 — blob offset 0xafc, 0x20 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0421i func_8018F960; C re-targeted by symbol address
 * (docs/evidence/room_m0107i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_8018FAE4(int a0, int a1, void *p)
{
    H(p, 0x10) = 256;
    H(p, 0x8) = 0;
    H(p, 0xA) = 0;
    H(p, 0xC) = 0;
    H(p, 0x12) = 1;
}
