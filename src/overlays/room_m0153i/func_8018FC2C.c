/* room_m0153i (PE.IMG room m0153i chunk 2, VRAM 0x8018EFE8)
 * func_8018FC2C — blob offset 0xc44, 0x1c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_8018FA90; C re-targeted by symbol address
 * (docs/evidence/room_m0153i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_8018FC2C(int a0, void *q)
{
    if (H(q, 0x2) == 58) {
        B(q, 0x1) = 2;
    }
}
