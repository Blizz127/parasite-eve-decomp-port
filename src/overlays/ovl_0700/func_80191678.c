/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80191678 — blob offset 0x2688, 0xC8 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80191678/REPORT.md).
 * Unlink slot n from the D_801E4A88 table and push it on the D_8019CBC0 free list. */

typedef struct { short prev; short next; } Link;
extern short D_8019CBC0;
extern short D_8019C9D0;
extern short D_8019C830[];
extern Link D_801E4A88[];
void func_80191678(short n)
{
    short f;
    D_801E4A88[D_801E4A88[n].prev].next = D_801E4A88[n].next;
    D_801E4A88[D_801E4A88[n].next].prev = D_801E4A88[n].prev;
    if (n == D_8019C9D0) {
        D_8019C9D0 = D_801E4A88[n].prev;
    }
    f = D_8019CBC0;
    D_801E4A88[n].prev = -1;
    D_801E4A88[n].next = -1;
    D_8019CBC0 = n;
    D_8019C830[n] = f;
}
