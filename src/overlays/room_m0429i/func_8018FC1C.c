/* room_m0429i (PE.IMG room m0429i chunk 2, VRAM 0x8018EFE8)
 * func_8018FC1C — blob offset 0xc34, 0xe0 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_8018FB78; C re-targeted by symbol address
 * (docs/evidence/room_m0429i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern int func_800DFF80();
extern short func_800DFFB8();
extern void func_800DFE94();
void func_8018FC1C(void *p, void *q, void *scratch)
{
    void *s;

    if (H(q, 0xA2) > 0) {
        s = P(q, 0x84);
        if (s != 0) {
            W(q, 0x60) = W(s, 0x28);
            W(q, 0x68) = W(s, 0x30);
        }
        H(p, 0x3A) = func_800DFFB8(H(p, 0x3A), (short)func_800DFF80((char *)q + 0x60, (char *)p + 0x28), H(q, 0xA2)) & 0xFFF;
        if (B(q, 0xA9) != 0 && W(q, 0x98) > 0x7FFFFFF) {
            H(q, 0xA2) = 0;
            H(q, 0xA4) = 0;
        }
    } else {
        func_800DFE94((char *)p + 0x28, (char *)p + 0x40, (char *)p + 0x38);
        H(p, 0x3A) = func_800DFFB8(H(p, 0x3A), H(p, 0x3A), H(q, 0xA4)) & 0xFFF;
    }
}
