/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80191580 — blob offset 0x2590, 0x5C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80191580/REPORT.md).
 * Record initialiser: scale shorts 0x1000, f24 = 1000, parent = &D_8019C340, links cleared. */

typedef struct Rec {
    short id;          /* 0x00 */
    short f02;         /* 0x02 */
    int f04;           /* 0x04 */
    short f08;         /* 0x08 */
    short f0A;
    short f0C;
    short f0E;
    short f10;         /* 0x10 */
    short f12;
    short f14;
    short f16;
    short f18;         /* 0x18 */
    short f1A;
    int f1C;           /* 0x1C */
    int f20;           /* 0x20 */
    int f24;           /* 0x24 */
    short f28;         /* 0x28 */
    short f2A;
    short f2C;
    short f2E;
    short f30;         /* 0x30 */
    short f32;         /* 0x32 */
    unsigned char pad34[0x28];
    short depth;       /* 0x5C */
    short f5E;
    void *parent;      /* 0x60 */
    struct Rec *next;  /* 0x64 */
    struct Rec *prev;  /* 0x68 */
} Rec;                 /* 0x6C */
typedef struct { short prev; short next; } Link;
extern unsigned char D_8019C340;
void func_80191580(Rec *r)
{
    r->f24 = 1000;
    r->f08 = 0x1000;
    r->f10 = 0x1000;
    r->f18 = 0x1000;
    r->f1C = 0;
    r->f20 = 0;
    r->f0A = 0;
    r->f0C = 0;
    r->f0E = 0;
    r->f12 = 0;
    r->f14 = 0;
    r->f16 = 0;
    r->f28 = 0;
    r->f2A = 0;
    r->f2C = 0;
    r->parent = &D_8019C340;
    r->next = 0;
    r->prev = 0;
}
