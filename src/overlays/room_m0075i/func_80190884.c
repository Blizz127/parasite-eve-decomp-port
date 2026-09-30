/* room_m0075i — func_80190884, blob offset 0x189C, 0x270 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * Two sprite descriptors at o+0xAC/o+0x78 from model matrices; h4 store before p. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct {
    void *p;
    short h4, h6, h8, hA, hC, hE;
    unsigned char b10, b11, b12, b13;
    MATRIX m;
} SPR;
extern short D_800942EC;
extern unsigned char D_80194370[];
extern unsigned char D_801940C8[];
extern unsigned char **func_800C2B50();
extern void func_800C5538();
extern void func_800C66C8();

#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

void func_80190884(void *a0, int a1, unsigned char *o)
{
    MATRIX mb;
    MATRIX ma;
    unsigned char **x;
    SPR *s;

    x = func_800C2B50();
    ma = *(MATRIX *)P(*x, 0x238);
    mb = *(MATRIX *)P(*x, 0x238);
    ma.t[0] = W(P(*x, 0x238), 0x134);
    ma.t[1] = D_800942EC;
    ma.t[2] = W(P(*x, 0x238), 0x13C);
    mb.t[0] = W(P(*x, 0x238), 0x1B4);
    mb.t[1] = D_800942EC;
    mb.t[2] = W(P(*x, 0x238), 0x1BC);
    s = (SPR *)(o + 0xAC);
    s->h4 = 0xA;
    s->p = D_80194370;
    s->h6 = 0;
    s->h8 = 0x190;
    s->hA = 0x12C;
    s->hC = 2;
    s->m = mb;
    s->b10 = 0x20;
    s->b11 = 1;
    s->b12 = 1;
    func_800C5538(s);
    s = (SPR *)(o + 0x78);
    s->h4 = 0xA;
    s->p = D_801940C8;
    s->h6 = 0;
    s->h8 = 0x190;
    s->hA = 0x12C;
    s->hC = 2;
    s->m = ma;
    s->b10 = 0x20;
    s->b11 = 1;
    s->b12 = 1;
    func_800C5538(s);
    func_800C66C8(a0, 0x576, &ma);
}
