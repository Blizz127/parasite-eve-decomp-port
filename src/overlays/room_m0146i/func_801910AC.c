/* room_m0146i (PE.IMG room m0146i chunk 2, VRAM 0x8018EFE8)
 * func_801910AC — blob offset 0x20c4, 0x14 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0137i func_8018FBDC; C re-targeted by symbol address
 * (docs/evidence/room_m0146i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_801910AC(int a0, int a1, void *p)
{
    H(p, 0x8) = 200;
    H(p, 0xA) = 128;
}
