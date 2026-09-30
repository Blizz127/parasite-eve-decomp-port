/* ovl_03D2 (PE.IMG title/boot overlay, VRAM 0x8018EFF0)
 * func_8018F7F0 — blob offset 0x800, 0x168 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_03D2-func_8018F7F0/REPORT.md).
 * Title sprite blend: for each row/4-pixel column of the task's image (t->img + 0x14, t->w*3/4 columns, t->h rows) scale source bytes by t->level (>>8) and keep the brighter of that and the destination (max blend); all-zero 4-byte groups are skipped. Levers: hand-unrolled 4x macro with src++ before the saved dst pointer and a '(v >= d) ? v : d' ternary (reproduces retail's extra register copy). */

typedef struct Task {
    struct Task *next;
    short x, y;
    short w, h;         /* 0x08, 0x0A */
    void *f0C;
    void *f10;
    int f14;
    unsigned char *img; /* 0x18 */
    int level;          /* 0x1C */
} Task;
#define BLEND()                      \
    v = (*src++ * k) >> 8;           \
    p = dst;                         \
    d = *dst++;                      \
    *p = (v >= d) ? v : d;
void func_8018F7F0(Task *t, unsigned char *dst, int stride)
{
    int k = t->level;
    int w = t->w * 3 / 4;
    int h;
    int x, y;
    int v;
    int d;
    unsigned char *p;
    unsigned char *src = t->img + 0x14;
    h = t->h;
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            if (*(int *)src != 0) {
                BLEND();
                BLEND();
                BLEND();
                BLEND();
            } else {
                src += 4;
                dst += 4;
            }
        }
        dst += stride * 4;
    }
}
