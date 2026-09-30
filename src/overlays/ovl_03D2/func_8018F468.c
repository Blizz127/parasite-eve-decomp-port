/* ovl_03D2 (PE.IMG subsystem overlay, VRAM 0x8018EFF0)
 * func_8018F468 — blob offset 0x478, 0x388 bytes. Profile era_o2_g0 (default).
 * Dirty-rect compositor: runs every sprite's update hook while accumulating the bounding box,
 * unions it with last frame's box in D_801D11C4, restores the background from the
 * D_80193254 image into the 0x8080 buffer and lets each sprite draw into it; stores the box back.
 * Levers (docs/evidence/ovl6-lane-2026-09-27/REPORT.md): miny/minx/maxy/maxx init order (saved-reg
 * order); int x0/y0/x1/y1 clamp locals; else-branch r.h last (cross-jumped with the then tail);
 * no Scr* local in the union block; `h = r.h` captured in the guard; image base in its own
 * statement (else the +0x14 folds into the symbol); draw args through a2/a1 locals. */
typedef struct { short x, y, w, h; } RECT;
typedef struct Spr {
    struct Spr *next;      /* 0x00 */
    short x, y, w, h;      /* 0x04 */
    void (*update)(struct Spr *);                 /* 0x0C */
    void (*draw)(struct Spr *, int *, int, int);  /* 0x10 */
} Spr;
typedef struct {
    unsigned char pad[0x70];
    RECT dirty;            /* 0x70 */
    short x, y, w, h;      /* 0x78 */
} Scr;
extern Spr *D_801D1370;
extern Scr *D_801D11C4;
extern unsigned char D_80193254[];
extern int D_80193258;

void func_8018F468(void)
{
    RECT r;
    Spr *s;
    Scr *c;
    int minx, miny, maxx, maxy;
    int v;
    int *src;
    int *dst;
    int i, j, n;
    int x0, y0, x1, y1;
    int h;
    unsigned char *b;
    int a1, a2;

    miny = 0x7FFF;
    minx = 0x7FFF;
    maxy = 0;
    maxx = 0;
    for (s = D_801D1370; s != 0; s = s->next) {
        if (s->update != 0) {
            s->update(s);
        }
        if (s->x < minx) minx = s->x;
        if (s->y < miny) miny = s->y;
        if (maxx < s->x + s->w) maxx = s->x + s->w;
        if (maxy < s->y + s->h) maxy = s->y + s->h;
    }
    if (D_801D11C4->w > 0) {
        x0 = minx;
        if (D_801D11C4->x < minx) x0 = D_801D11C4->x;
        r.x = x0;
        y0 = miny;
        if (D_801D11C4->y < miny) y0 = D_801D11C4->y;
        r.y = y0;
        x1 = maxx;
        if (maxx < D_801D11C4->x + D_801D11C4->w) x1 = D_801D11C4->x + D_801D11C4->w;
        r.w = x1 - x0;
        y1 = maxy;
        if (maxy < D_801D11C4->y + D_801D11C4->h) y1 = D_801D11C4->y + D_801D11C4->h;
        r.h = y1 - y0;
    } else {
        r.w = maxx - minx;
        r.x = minx;
        r.y = miny;
        r.h = maxy - miny;
    }
    if (r.w > 0 && (h = r.h) > 0) {
        b = D_80193254 + D_80193258;
        v = ((r.y - 20) * 320 + r.x) * 3;
        src = (int *)b + (v / 4 + 5);
        dst = (int *)((unsigned char *)D_801D11C4 + 0x8080);
        n = (r.w * 3) / 4;
        for (i = 0; i < h; i++) {
            for (j = 0; j < n; j++) {
                *dst++ = *src++;
            }
            src += 0xF0 - n;
        }
        for (s = D_801D1370; s != 0; s = s->next) {
            if (s->draw != 0) {
                a2 = r.w - s->w;
                a1 = r.w * (s->y - r.y) + (s->x - r.x);
                s->draw(s, &((int *)D_801D11C4)[(a1 * 3) / 4 + 0x2020], (a2 * 3) / 4, (r.w * 3) / 4);
            }
        }
    }
    c = D_801D11C4;
    c->w = maxx - minx;
    c->x = minx;
    c->y = miny;
    c->h = maxy - miny;
    c->dirty = r;
}
