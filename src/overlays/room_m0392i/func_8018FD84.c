/* room_m0392i (PE.IMG room m0392i chunk 2, VRAM 0x8018EFE8)
 * func_8018FD84 — blob offset 0xd9c, 0x14 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0137i func_8018FBDC; C re-targeted by symbol address
 * (docs/evidence/room_m0392i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_8018FD84(int a0, int a1, void *p)
{
    H(p, 0x8) = 200;
    H(p, 0xA) = 128;
}
