/* room_m0065i (PE.IMG room m0065i chunk 2, VRAM 0x8018EFE8)
 * func_8018F128 — blob offset 0x140, 0x84 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0034i func_8018F1B8; C re-targeted by symbol address
 * (docs/evidence/room_m0065i-ports-2026-09-23/REPORT.md). */

#define P(o, x) (*(void **)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
extern unsigned int func_800C6CE0();
int func_8018F128(unsigned char *o)
{
    int s = 4;

    o[0] = s;
    if (func_800C6CE0(o) >= 2) {
        W(P(P(o, 0x8), 0x0), 0x0) &= 0xC0FFFFFF;
        *(unsigned char *)P(P(P(o, 0x8), 0x0), 0x18) = s;
    }
    return 0;
}
