/* ovl_0457 (PE.IMG credits/XA overlay, VRAM 0x80120D00)
 * func_80124704 — blob offset 0x3A04, 0xF0 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0457-func_80124704/REPORT.md).
 * Queue the 96x24 credits-title sprite (DR_TPAGE + semi-transparent SPRT, grey level c) from the D_80172CB8 prim cursor into ot[11] of D_80172CB4, advancing the cursor by one 0x1C-byte pair. */

typedef struct {
    unsigned addr : 24;
    unsigned len : 8;
} P_TAG;
typedef struct {
    unsigned long tag;
    unsigned long code[1];
} DR_TPAGE;
typedef struct {
    unsigned long tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short w, h;
} SPRT;
typedef struct {
    DR_TPAGE tp;
    SPRT s;
} TSprite;
#define setlen(p, _len) (((P_TAG *)(p))->len = (unsigned char)(_len))
#define setcode(p, _code) (((SPRT *)(p))->code = (unsigned char)(_code))
#define setSprt(p) setlen(p, 4), setcode(p, 0x64)
extern unsigned short D_80172CA0;
extern unsigned short D_80172CA2;
extern TSprite *D_80172CB8;
extern unsigned long *D_80172CB4;
extern void func_80077C84();
extern void func_80077B04();
extern void func_80077CB4();
extern void func_80077AC4();
void func_80124704(unsigned char c)
{
    TSprite *o = D_80172CB8;
    func_80077C84(o, 0, 0, D_80172CA0);
    setSprt(&o->s);
    func_80077B04(&o->s, 1);
    func_80077CB4(o, &o->s);
    o->s.x0 = 0x78;
    o->s.y0 = 0x6E;
    o->s.u0 = 0;
    o->s.v0 = 0x80;
    o->s.r0 = c;
    o->s.g0 = c;
    o->s.b0 = c;
    o->s.w = 0x60;
    o->s.h = 0x18;
    o->s.clut = D_80172CA2;
    func_80077AC4(D_80172CB4 + 11, o);
    D_80172CB8++;
}
