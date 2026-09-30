/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80190998 — blob offset 0x19A8, 0x154 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80190998/REPORT.md).
 * Initialise the record pool (199 records + root D_8019C340), the D_8019C830 free list, the three depth sentinels (D_8019CC58 <-> D_801EA5E8) and the D_801E4A88 link table. Sentinel store order (prev link first) is load-bearing (4 words). */

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
extern Rec D_801E4E00[];
extern Rec D_8019C340;
extern Rec D_8019CC58[];
extern Rec D_801EA5E8[];
extern short D_8019CBC0;
extern short D_8019C9D0;
extern short D_8019C830[];
extern Link D_801E4A88[];
extern void func_80191580();
void func_80190998(void)
{
    int i;
    for (i = 1; i < 200; i++) {
        func_80191580(&D_801E4E00[i]);
    }
    func_80191580(&D_8019C340);
    D_8019C340.depth = 0;
    D_8019CBC0 = 0;
    for (i = 0; i < 200; i++) {
        D_8019C830[i] = i + 1;
    }
    for (i = 0; i < 3; i++) {
        D_801EA5E8[i].prev = &D_8019CC58[i];
        D_8019CC58[i].next = &D_801EA5E8[i];
        D_801EA5E8[i].next = 0;
        D_8019CC58[i].prev = 0;
    }
    for (i = 0; i < 200; i++) {
        D_801E4A88[i].prev = -1;
        D_801E4A88[i].next = -1;
    }
    D_8019C9D0 = 200;
    D_801E4A88[200].prev = -1;
    D_801E4A88[200].next = 201;
    D_801E4A88[201].prev = 200;
    D_801E4A88[201].next = -1;
}
