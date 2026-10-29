/* ovl_03D2 func_8018F958 — blob offset 0x968, 0x268 bytes. Profile era_o2_g0 (default).
 * Two-pass title-sprite max blend (sibling of func_8018F7F0): pass 1 walks 24 rows of a
 * 3-byte-pixel image (t->img + 0x14), width clamped to 0x54 columns, writing
 * dst = max(dst, (src * level) >> 8); pass 2 walks the image backward (column stride
 * -0xFC, row step -3) from t->img + 0x17B1 over 0x54 - y0 columns.
 * Levers: '*D++ = (v >= d) ? v : d' post-increment store (fresh address temp per byte, so
 * cse keeps the w-based loads in pass 2); 'n = 0x54; if (r < 0x55) n = r;' polarity;
 * comma-expression 'x = 0x54 - row' so loop.c reduces the 84-row giv without folding y0;
 * volatile img/y0 fields keep retail's y0-before-img load order. */
typedef struct {
    int pad[6];
    unsigned char *volatile img; /* 0x18 */
    int level;                   /* 0x1C */
    volatile int y0;             /* 0x20 */
} Blend;
#define BLEND(S, D)                  \
    v = ((S) * k) >> 8;              \
    d = *D;                          \
    *D++ = (v >= d) ? v : d;
void func_8018F958(Blend *a0, unsigned char *a1, int a2, int a3)
{
    unsigned char *dst = a1;
    int y0 = a0->y0;
    unsigned char *src = a0->img + 0x14;
    int k = a0->level;
    int r = y0 + 0x18;
    int v0 = 0x3C - y0;
    int row, i, n, v, d, t, x;
    unsigned char *q, *w;

    if (v0 >= 0)
        dst += (a3 * v0) << 2;
    for (row = 0; row < 0x18; row++) {
        n = 0x54;
        if (r < 0x55)
            n = r;
        for (i = 0; i < n; i++) {
            BLEND(*src++, dst)
            BLEND(*src++, dst)
            BLEND(*src++, dst)
        }
        t = (0x54 - n) * 3;
        src += t;
        dst += (a2 << 2) + t;
        r--;
    }
    src = a0->img + 0x17B1;
    dst = a1 + y0 * 3;
    for (row = 0; row < 0x54 - y0; row++) {
        q = src;
        w = dst;
        for (i = 0; ((x = 0x54 - row, x - y0 < 0x19) ? (i < x - y0) : (i < 0x18)); i++) {
            BLEND(q[0], w)
            BLEND(q[1], w)
            BLEND(q[2], w)
            q -= 0xFC;
        }
        src -= 3;
        dst += a3 << 2;
    }
}
