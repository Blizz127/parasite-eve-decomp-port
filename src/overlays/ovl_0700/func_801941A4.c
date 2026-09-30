/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_801941A4 — blob offset 0x51B4, 0x158 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_801941A4/REPORT.md).
 * Full-screen semi-transparent grey POLY_G4 (colour c) plus a DR_TPAGE (tpage 0x40), both addPrim'd into ot[0] of the current D_8019C9C0 buffer, advancing primptr. */

typedef struct {
    unsigned addr : 24;
    unsigned len : 8;
} P_TAG;
typedef struct {
    unsigned long tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char r1, g1, b1, pad1;
    short x1, y1;
    unsigned char r2, g2, b2, pad2;
    short x2, y2;
    unsigned char r3, g3, b3, pad3;
    short x3, y3;
} POLY_G4;
typedef struct {
    unsigned char *primptr;
    unsigned long *ot;
} DB;
extern DB *D_8019C9C0;
extern void func_80077BC4();
extern void func_80077B04();
extern void func_80077C84();

#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (unsigned long)(_addr))
#define getaddr(p) (unsigned long)(((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)

void func_801941A4(unsigned char c)
{
    POLY_G4 *p = (POLY_G4 *)D_8019C9C0->primptr;
    func_80077BC4(p);
    func_80077B04(p, 1);
    p->r0 = c; p->g0 = c; p->b0 = c;
    p->r1 = c; p->g1 = c; p->b1 = c;
    p->r2 = c; p->g2 = c; p->b2 = c;
    p->r3 = c; p->g3 = c; p->b3 = c;
    p->x0 = 0; p->y0 = 0;
    p->x1 = 320; p->y1 = 0;
    p->x2 = 0; p->y2 = 240;
    p->x3 = 320; p->y3 = 240;
    addPrim(D_8019C9C0->ot, p);
    p = (POLY_G4 *)((unsigned char *)p + 0x24);
    D_8019C9C0->primptr = (unsigned char *)p;
    func_80077C84(p, 0, 0, 0x40);
    addPrim(D_8019C9C0->ot, p);
    p = (POLY_G4 *)((unsigned char *)p + 8);
    D_8019C9C0->primptr = (unsigned char *)p;
}
