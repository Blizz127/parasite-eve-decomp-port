/* room_m0034i (PE.IMG room m0034i chunk 2, VRAM 0x8018EFE8)
 * func_8018F380 — blob offset 0x398, 0x3c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_8018F5C8; C re-targeted by symbol address
 * (docs/evidence/room_m0034i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern int *func_800C2B10();
void func_8018F380(int a0, int a1, void *p)
{
    H(p, 0x0) = 0;
    H(p, 0x2) = *func_800C2B10(4);
}
