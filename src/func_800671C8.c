/*
 * func_800671C8 — translate a geometry record: x = +8 + dx and y = +0xA + dy
 * clamped (as shorts) into [+0x10,+0x12] / [+0x14,+0x16] and stored at
 * +0xC/+0xE; the 12-bit depth field (bits 8..19 of word 0) becomes
 * dz + (word0 >> 20). Returns 0.
 *
 * ROM: era gcc-2.7.2-psx -O2 -G0 (default). The `short` locals are what give
 * retail its unused 0x20-byte frame (2.7.2 stack-temps a short if/else merge).
 */
typedef struct {
    unsigned int w0;
    int w4;
    unsigned short x, y;
    short cx, cy;
    short minx, maxx, miny, maxy;
} Geo;
int func_800671C8(Geo *g, int dx, int dy, int dz) {
    short x = g->x + dx;
    short y = g->y + dy;
    int z = dz + (g->w0 >> 20);
    if (x < g->minx) x = g->minx; else if (x > g->maxx) x = g->maxx;
    if (y < g->miny) y = g->miny; else if (y > g->maxy) y = g->maxy;
    g->cx = x;
    g->cy = y;
    g->w0 = (g->w0 & 0xFFF000FF) | ((z & 0xFFF) << 8);
    return 0;
}
