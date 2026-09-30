/* room_m0063i (PE.IMG room m0063i chunk 2, VRAM 0x8018EFE8)
 * func_8018FD5C — blob offset 0xd74, 0x78 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0358i func_80193250; C re-targeted by symbol address
 * (docs/evidence/room_m0063i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
int func_8018FD5C(void *o)
{
    void *s;

    B(o, 0x0) = 4;
    B(o, 0x3) = 0;
    W(P(o, 0x8), 0x98) &= 0xFFFEFFFF;
    H(P(o, 0x8), 0x250) &= 0xFBFF;
    s = P(P(o, 0x8), 0x18C);
    if (s != 0 && P(s, 0x0) != 0) {
        *(unsigned char *)P(P(s, 0x0), 0x18) = 4;
    }
    return 0;
}
