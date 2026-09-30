/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_801917BC — blob offset 0x27CC, 0x78 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_801917BC/REPORT.md).
 * Insert record r under parent at depth parent->depth+1 into that depth's sentinel list (D_8019CC58[d]). */

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
extern Rec D_8019CC58[];
void func_801917BC(Rec *r, Rec *parent)
{
    short d;
    Rec *h;
    r->parent = parent;
    d = r->depth = parent->depth + 1;
    h = D_8019CC58[d].next;
    r->next = h;
    r->prev = h->prev;
    D_8019CC58[d].next->prev = r;
    D_8019CC58[d].next = r;
}
