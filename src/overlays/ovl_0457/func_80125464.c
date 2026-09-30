/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80125464 — blob offset 0x4764, 0x140 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_80125464/REPORT.md).
 * Additive glow over the credits image: for rows 0x6E..0x85, each 16-bit mask word selects per-pixel intensity nibbles (D_80172C9C table, & 0x1F << 3) added to RGB24 pixels with 0xFF clamps. Levers: load r/g/b into locals before the add, 'r, b, g' declaration order, $14/$13 pins for the row/line bases. */

extern unsigned char *D_80172C98;
extern unsigned short *D_80172C9C;
extern int D_80172CBC;
typedef struct { unsigned char *p; int pad[3]; } Img;
extern Img D_80125B88[];
void func_80125464(void)
{
    short y, x, k;
    register unsigned short *row asm("$14") = (unsigned short *)(D_80172C98 + 0x4000);
    register unsigned char *line asm("$13") = D_80125B88[D_80172CBC].p + 0x19DAC;
    unsigned short *m;
    unsigned char *p;
    unsigned int bits, add, r, b, g;
    for (y = 0x6E; y < 0x86; y++) {
        m = row;
        p = line;
        for (x = 0; x < 0x18; x++) {
            bits = *m;
            if (bits != 0) {
                for (k = 0; k < 4; k++) {
                    if (bits & 0xF) {
                        g = p[2];
                        b = p[1];
                        r = p[0];
                        add = (D_80172C9C[bits & 0xF] & 0x1F) << 3;
                        g += add;
                        if (g > 0xFF) g = 0xFF;
                        b += add;
                        if (b > 0xFF) b = 0xFF;
                        r += add;
                        if (r > 0xFF) r = 0xFF;
                        p[2] = g;
                        p[1] = b;
                        p[0] = r;
                    }
                    bits >>= 4;
                    p += 3;
                }
            } else {
                p += 12;
            }
            m++;
        }
        row += 0x40;
        line += 0x3C0;
    }
}
