/* room_m0429i (PE.IMG room m0429i chunk 2, VRAM 0x8018EFE8)
 * func_80192688 — blob offset 0x36a0, 0x2c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0269i func_8018F160; C re-targeted by symbol address
 * (docs/evidence/room_m0429i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_801926B4();
void func_80192688(void *o)
{
    if (B(P(o, 0x8), 0xE) == 16) {
        P(o, 0xC) = func_801926B4;
    }
}
