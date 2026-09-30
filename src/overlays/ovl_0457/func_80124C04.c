/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80124C04 — blob offset 0x3F04, 0x33C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl14-lane-2026-09-28/REPORT.md).
 * 16x16 4bpp glyph blit into the current 320x224 BGR24 frame buffer with
 * additive saturation.  Levers: g/r/b declared g-first (allocation order),
 * block-local k, named lo/hi mask temps (sched tie order), src pinned $11. */

typedef struct {
    unsigned char *buf;
    int pad[3];
} Screen;

extern Screen D_80125B88[];
extern int D_80172CBC;
extern unsigned short *D_80172C98;
extern unsigned short *D_80172C9C;
extern int func_80123B1C(int c);

/* One 4-bit glyph texel: additive-saturate its palette intensity into a BGR24 pixel. */
#define PIX(v, sh)                                                  \
    if ((v) != 0) {                                                 \
        unsigned int k = (D_80172C9C[(v) >> (sh)] & 0x1F) << 3;     \
        r = q[2];                                                   \
        g = q[1];                                                   \
        b = q[0];                                                   \
        r += k;                                                     \
        if (r >= 0x100) r = 0xFF;                                   \
        g += k;                                                     \
        if (g >= 0x100) g = 0xFF;                                   \
        b += k;                                                     \
        if (b >= 0x100) b = 0xFF;                                   \
        q[2] = r;                                                   \
        q[1] = g;                                                   \
        q[0] = b;                                                   \
    }                                                               \
    q += 3;

void func_80124C04(short x, short y, unsigned char ch)
{
    unsigned char *row;
    unsigned char *q;
    register unsigned short *src asm("$11");
    unsigned short *s;
    unsigned int g, r, b;
    unsigned int m;
    int gl;
    short yy;
    short i;

    row = D_80125B88[D_80172CBC].buf;
    gl = func_80123B1C(ch);
    src = D_80172C98;
    {
        int lo = gl & 0xF;
        int hi = gl & 0xF0;
        src += lo * 4 + hi * 64;
    }
    row += x * 3 + y * 960;
    for (yy = y; yy < y + 16; src += 64, yy++, row += 960) {
        s = src;
        q = row;
        if (yy < 0) continue;
        if (yy >= 0xE0) break;
        for (i = 0; i < 4; i++, s++) {
            m = *s;
            if (m != 0) {
                PIX(m & 0xF, 0)
                PIX(m & 0xF0, 4)
                PIX(m & 0xF00, 8)
                PIX(m & 0xF000, 12)
            } else {
                q += 12;
            }
        }
    }
}
