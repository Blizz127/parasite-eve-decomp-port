/* Reader leaf: pins D_8009D240 / D_8009D260 as pointers by proof.
 * VRAM 0x8008AB1C / file 0x7B31C / size 0x80.
 * Era path: gcc-2.7.2-psx -O1 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * Type evidence (live unit 778F0.s):
 *   D_8009D240: lw; sll/addu; lhu — pointer to unsigned short elements.
 *   D_8009D260: lw; addu with halfword offset (unscaled) — unsigned char *
 *               base; result stored through void ** outs.
 */
extern unsigned short *D_8009D240;
extern unsigned char *D_8009D260;

void func_8008AB1C(void **out0, void **out1, unsigned int a2) {
    unsigned int v;
    unsigned char *result;

    a2 = (a2 & 0x3FFu) << 1;

    v = D_8009D240[a2];
    if (v != 0xFFFF) {
        result = D_8009D260;
        result += v;
    } else {
        result = 0;
    }
    a2 = a2 + 1;
    *out0 = result;

    v = D_8009D240[a2];
    if (v != 0xFFFF) {
        result = D_8009D260;
        result += v;
    } else {
        result = 0;
    }
    *out1 = result;
}
