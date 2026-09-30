/* func_800E051C — EXE, VRAM 0x800E051C, file offset 0xD0D1C, 0x2EC bytes (187 words).
 * Park-effect glow sprite: one camera-facing POLY_FT4 (code 0x2E, semi-transparent) built from
 * the parameter block its callers (func_800E026C / func_800E03A0) publish in the scratchpad,
 * sized (s<<5)*scale / ((sz<<4)+scale) >> 1 and linked at OT[(sz>>2)-8].
 * Profile era_o2_g0_expand_div (ASPSX div guards; -O2 -G0).
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros
 * (gte_SetRotMatrix, gte_SetTransMatrix, gte_ldv0, gte_rtps, gte_stsxy, gte_stsz) — count
 * separately from plain C.
 * Hand-written in this repo (khasinski's vendored tree has no C for this body).
 * Evidence: docs/evidence/func-800E051C/REPORT.md */
/* Psy-Q inline_c.h-shaped GTE macros (inline COP2 asm; count separately from plain C). */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14,0(%0)" : : "r"(r0) : "memory")
#define gte_stsz(r0) __asm__ volatile ("swc2 $19,0(%0)" : : "r"(r0) : "memory")

typedef struct { unsigned addr : 24; unsigned len : 8; } P_TAG;
#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (unsigned int)(_addr))
#define getaddr(p) (unsigned int)(((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)
#define setlen(p, _len) (((P_TAG *)(p))->len = (unsigned char)(_len))
typedef struct { char *ordering[2]; char *other_buffers[6]; char *packets[2]; } RenderBufferPrefix;
typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short x1, y1;
    unsigned char u1, v1;
    unsigned short tpage;
    short x2, y2;
    unsigned char u2, v2;
    unsigned short pad1;
    short x3, y3;
    unsigned char u3, v3;
    unsigned short pad2;
} POLY_FT4;
typedef struct {
    short sx, sy;
    int sz;
    int pad08[4];
    unsigned char r, g, b, pad1b;
    unsigned char u, v;
    unsigned short clut;
    unsigned short pad20;
    unsigned short tpage;
    short vec[4];
    int size;
    int pad30;
    void *mat;
} Scratch;

extern int D_8009CDD8;
extern int D_8009CDDC;
extern RenderBufferPrefix D_800B0E38;
extern int *D_800BCFA8;

#define S ((Scratch *)0x1F800000)

void func_800E051C(void)
{
    POLY_FT4 *p;
    int pos;
    int h;
    Scratch *s = S;

    pos = D_8009CDD8;
    D_8009CDD8 = pos + 0x28;
    p = (POLY_FT4 *)(D_800B0E38.packets[D_8009CDDC] + pos);
    gte_SetRotMatrix(s->mat);
    gte_SetTransMatrix(s->mat);
    gte_ldv0(s->vec);
    gte_rtps();
    setlen(p, 9);
    p->code = 0x2E;
    p->r0 = s->r;
    p->g0 = s->g;
    p->b0 = s->b;
    p->clut = s->clut;
    p->tpage = s->tpage;
    p->u0 = s->u;
    p->v0 = s->v;
    p->u1 = s->u + 16;
    p->v1 = s->v;
    p->u2 = s->u;
    p->v2 = s->v + 16;
    p->u3 = s->u + 16;
    p->v3 = s->v + 16;
    gte_stsxy(&s->sx);
    gte_stsz(&s->sz);
    h = ((s->size << 5) * *D_800BCFA8) / ((s->sz << 4) + *D_800BCFA8) >> 1;
    p->x0 = s->sx - h;
    p->y0 = s->sy - h;
    p->x1 = s->sx + h;
    p->y1 = s->sy - h;
    p->x2 = s->sx - h;
    p->y2 = s->sy + h;
    p->x3 = s->sx + h;
    p->y3 = s->sy + h;
    addPrim((unsigned int *)D_800B0E38.ordering[D_8009CDDC] + ((s->sz >> 2) - 8), p);
}
