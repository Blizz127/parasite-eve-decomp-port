/*
 * func_80067A78 — rebase the D_800B1624 header's screen origin
 * (+0x38 = +0x2C - (D_800BCF8C - 0xA0), +0x3A = +0x2E - (D_800BCF8E - 0x70)),
 * then walk its 0x38-byte entry table (count at +6, offset at +0x14) and call
 * func_80067294 on every entry with flag bit 1 set whose +0x24 byte equals
 * D_800BCFFD.
 *
 * ROM: era gcc-2.7.2-psx -O2 -G0 (default). Load-bearing: `short` casts on the
 * (D - const) terms (else cc1 reassociates to x + const - D), `b +=` for the
 * table base and an indexed `p = b + i * 0x38` (a walking pointer allocates
 * the base straight into $s0: 36-42 words).
 */
extern unsigned char *D_800B1624;
extern unsigned short D_800BCF8C;
extern unsigned short D_800BCF8E;
extern unsigned char D_800BCFFD;
extern void func_80067294(unsigned char *);
int func_80067A78(void) {
    unsigned char *b = D_800B1624;
    unsigned char *p;
    int n, i;
    *(short *)(b + 0x38) = *(short *)(b + 0x2C) - (short)(D_800BCF8C - 0xA0);
    *(short *)(b + 0x3A) = *(short *)(b + 0x2E) - (short)(D_800BCF8E - 0x70);
    n = *(unsigned short *)(b + 6);
    b += *(int *)(b + 0x14);
    for (i = 0; i < n; i++) {
        p = b + i * 0x38;
        if ((p[0] & 2) && p[0x24] == D_800BCFFD) {
            func_80067294(p);
        }
    }
    return 0;
}
