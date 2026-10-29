/* func_800D2B58 — EXE, VRAM 0x800D2B58, file offset 0xC3358, 0x5BC bytes.
 * Streak: a gradient LINE_G2 between two projected points plus a textured POLY_GT4
 * glow quad oriented along the line (ratan2), fixed UVs 0x90..0x9F x 0xD0..0xDF,
 * clut (0x20, 0x1D6), optional DR_MODE semi-transparency.  Retail quirk kept: the
 * second point's blue channel scales by the FIRST brightness (b0), not b1.
 * Profile era_o2_g0 (default).
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count separately
 * from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only).
 * Evidence: docs/evidence/func-800D2B58/REPORT.md */
/* Psy-Q inline_c.h-shaped GTE macros (inline COP2 asm; count separately from plain C). */
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)" : : "r"(r0))
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14,0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ( \
    "mfc2 $12,$19\n\tnop\n\tsra $12,$12,2\n\tsw $12,0(%0)" : : "r"(r0) : "$12", "memory")

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char r1, g1, b1, p1;
    short x1, y1;
} LINE_G2;
typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0; unsigned short clut;
    unsigned char r1, g1, b1, p1;
    short x1, y1;
    unsigned char u1, v1; unsigned short tpage;
    unsigned char r2, g2, b2, p2;
    short x2, y2;
    unsigned char u2, v2; unsigned short pad2;
    unsigned char r3, g3, b3, p3;
    short x3, y3;
    unsigned char u3, v3; unsigned short pad3;
} POLY_GT4;
typedef struct { unsigned addr : 24; unsigned len : 8; } P_TAG;
#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (unsigned int)(_addr))
#define getaddr(p) (unsigned int)(((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)
#define setSemiTrans(p, abe) ((abe) ? ((p)->code |= 0x2) : ((p)->code &= ~0x2))

typedef struct { char *ordering[2]; char *other_buffers[6]; char *packets[2]; } RenderBufferPrefix;
extern RenderBufferPrefix D_800B0E38;
extern int D_8009CDD8;
extern int D_8009CDDC;
extern unsigned short D_800F3374;
extern unsigned short D_800E2852;
extern int func_80077A64();
extern int func_80077AA4();
extern void func_80077AC4();
extern void func_80077C84();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80079FB4();

void func_800D2B58(SVECTOR *p0, SVECTOR *p1, unsigned char *c0, unsigned char *c1,
                   int b0, int b1, int abr)
{
    int otz;
    int otz1;
    LINE_G2 *line;
    POLY_GT4 *quad;
    unsigned int *ot;
    char *mode;
    int a;
    int cs, sn, cl, sl;

    line = (LINE_G2 *)(D_800B0E38.packets[D_8009CDDC] + D_8009CDD8);
    D_8009CDD8 += sizeof(LINE_G2);
    quad = (POLY_GT4 *)(D_800B0E38.packets[D_8009CDDC] + D_8009CDD8);
    D_8009CDD8 += sizeof(POLY_GT4);
    gte_ldv0(p0);
    gte_rtps();
    line->r0 = c0[0] * b0 / 128;
    line->g0 = c0[1] * b0 / 128;
    line->b0 = c0[2] * b0 / 128;
    gte_stszotz(&otz);
    gte_stsxy(&line->x0);
    ((unsigned char *)line)[3] = 4;
    line->code = 0x50;
    gte_ldv0(p1);
    gte_rtps();
    line->r1 = c1[0] * b1 / 128;
    line->g1 = c1[1] * b1 / 128;
    line->b1 = c1[2] * b0 / 128;
    gte_stszotz(&otz1);
    gte_stsxy(&line->x1);
    otz = (otz + otz1) / 2 - D_800F3374;
    if ((unsigned int)otz >= 0x1000) {
        return;
    }
    ot = (unsigned int *)((otz << 2) + (int)D_800B0E38.ordering[D_8009CDDC]);
    if (abr != 0xFF) {
        mode = D_800B0E38.packets[D_8009CDDC] + D_8009CDD8;
        D_8009CDD8 += 8;
        func_80077C84(mode, 0, 1, func_80077A64(0, abr, 0, 0) & 0xFFFF);
        if (line != 0) {
            setSemiTrans(line, 1);
            addPrim(ot, line);
        }
        addPrim(ot, mode);
    } else if (line != 0) {
        addPrim(ot, line);
    }
    a = func_80079FB4(line->y1 - line->y0, line->x1 - line->x0);
    cl = func_80077CF4(-a) / 512;
    sl = func_80077DC4(-a) / 512;
    sn = func_80077DC4(a) / 256;
    cs = func_80077CF4(a) / 256;
    ((unsigned char *)quad)[3] = 0xC;
    quad->code = 0x3C;
    if (abr == 0xFF) {
        quad->code = 0x3C;
        quad->tpage = D_800E2852;
    } else {
        quad->code = 0x3E;
        quad->tpage = D_800E2852 | func_80077A64(0, abr, 0, 0);
    }
    quad->clut = func_80077AA4(0x20, 0x1D6);
    quad->x0 = line->x0 - sn + cl;
    quad->y0 = line->y0 - cs + sl;
    quad->x1 = line->x0 - sn - cl;
    quad->y1 = line->y0 - cs - sl;
    quad->x2 = line->x1 + sn + cl;
    quad->y2 = line->y1 + cs + sl;
    quad->x3 = line->x1 + sn - cl;
    quad->y3 = line->y1 + cs - sl;
    quad->u0 = 0x90;
    quad->u2 = 0x90;
    quad->v0 = 0xD0;
    quad->v1 = 0xD0;
    quad->u1 = 0x9F;
    quad->u3 = 0x9F;
    quad->v2 = 0xDF;
    quad->v3 = 0xDF;
    quad->r0 = line->r0; quad->g0 = line->g0; quad->b0 = line->b0;
    quad->r1 = line->r0; quad->g1 = line->g0; quad->b1 = line->b0;
    quad->r2 = line->r1; quad->g2 = line->g1; quad->b2 = line->b1;
    quad->r3 = line->r1; quad->g3 = line->g1; quad->b3 = line->b1;
    func_80077AC4(D_800B0E38.ordering[D_8009CDDC] + (otz << 2), quad);
}
