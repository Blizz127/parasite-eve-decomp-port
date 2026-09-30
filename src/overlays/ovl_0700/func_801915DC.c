/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_801915DC — blob offset 0x25EC, 0x9C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_801915DC/REPORT.md).
 * Pop a slot from the D_8019CBC0 free list and link it after D_8019C9D0 in the D_801E4A88 prev/next table. */

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
extern short D_8019CBC0;
extern short D_8019C9D0;
extern short D_8019C830[];
extern Link D_801E4A88[];
short func_801915DC(void)
{
    short n = D_8019CBC0;
    short p = D_8019C9D0;
    short f;
    D_801E4A88[n].prev = p;
    f = D_8019C830[n];
    D_801E4A88[n].next = D_801E4A88[p].next;
    D_801E4A88[p].next = n;
    D_8019C9D0 = n;
    D_8019CBC0 = f;
    D_801E4A88[D_801E4A88[n].next].prev = n;
    return n;
}
