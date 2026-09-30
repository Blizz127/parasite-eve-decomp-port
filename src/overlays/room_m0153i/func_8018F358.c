/* room_m0153i (PE.IMG room m0153i chunk 2, VRAM 0x8018EFE8)
 * func_8018F358 — blob offset 0x370, 0x60 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_8018F1BC; C re-targeted by symbol address
 * (docs/evidence/room_m0153i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern unsigned int func_800C6CE0();
int func_8018F358(unsigned char *o)
{
    o[0] = 4;
    if (func_800C6CE0(o) >= 2) {
        W(P(P(o, 0x8), 0x0), 0x0) &= 0xC0FFFFFF;
    }
    return 0;
}
