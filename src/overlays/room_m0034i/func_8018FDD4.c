/* room_m0034i (PE.IMG room m0034i chunk 2, VRAM 0x8018EFE8)
 * func_8018FDD4 — blob offset 0xdec, 0x10 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_8019001C; C re-targeted by symbol address
 * (docs/evidence/room_m0034i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_8018FDD4(int a0, int a1, void *p)
{
    B(p, 0x2) = 0;
    H(p, 0x4) = 128;
}
