/* room_m0022i (PE.IMG room m0022i chunk 2, VRAM 0x8018EFE8)
 * func_8018FE98 — blob offset 0xeb0, 0x9c bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room_m0022i-batch-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
int func_8018FE98(void *o)
{
    void *q;
    void *s = (char *)o + 0xC;
    unsigned char *p;

    B(o, 0x0) = 4;
    B(o, 0x3) = 0;
    if (B(o, 0xB8) == 0) {
        if (H(o, 0x14) != 0) {
            q = *(void **)P(o, 0x8);
            if (q != 0) {
                *(unsigned char *)P(q, 0x18) = 4;
            }
        }
    } else {
        q = *(void **)P(o, 0x8);
        if (q != 0) {
            p = P(q, 0x18);
            if (*p == 1) {
                *p = 4;
            }
        }
    }
    if (P(s, 0x4) != 0) {
        *(int *)P(s, 0x4) = 0;
    }
    return 0;
}
