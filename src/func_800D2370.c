/* func_800D2370 — EXE, VRAM 0x800D2370, file offset 0xC2B70, 0x48C bytes.
 * Shaded textured ribbon: one POLY_GT4 along the rotated local Z axis (length, width, UV rect, two end colours).
 * Profile era_o2_g0.
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count separately
 * from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only).
 * Evidence: docs/evidence/func-800D2370/REPORT.md */
/* Psy-Q inline_c.h-shaped GTE macros (inline COP2 asm; count separately from plain C). */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)" : : "r"(r0))
#define gte_ldv3(r0, r1, r2) __asm__ volatile ( \
    "lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)\n\tlwc2 $2,0(%1)\n\tlwc2 $3,4(%1)\n\t" \
    "lwc2 $4,0(%2)\n\tlwc2 $5,4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_rt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_rtpt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A280030")
#define gte_stlvnl(r0) __asm__ volatile ( \
    "swc2 $25,0(%0)\n\tswc2 $26,4(%0)\n\tswc2 $27,8(%0)" : : "r"(r0) : "memory")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14,0(%0)" : : "r"(r0) : "memory")
#define gte_stsxy3(r0, r1, r2) __asm__ volatile ( \
    "swc2 $12,0(%0)\n\tswc2 $13,0(%1)\n\tswc2 $14,0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")
#define gte_stopz(r0) __asm__ volatile ("swc2 $24,0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ( \
    "mfc2 $12,$19\n\tnop\n\tsra $12,$12,2\n\tsw $12,0(%0)" : : "r"(r0) : "$12", "memory")
#define gte_avsz3() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B58002D")
#define gte_SetGeomScreen(r0) __asm__ volatile ("ctc2 %0,$26" : : "r"(r0))

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { unsigned char b[4]; } B4;
typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    unsigned char r1, g1, b1, p1;
    short x1, y1;
    unsigned char u1, v1;
    unsigned short tpage;
    unsigned char r2, g2, b2, p2;
    short x2, y2;
    unsigned char u2, v2;
    unsigned short pad2;
    unsigned char r3, g3, b3, p3;
    short x3, y3;
    unsigned char u3, v3;
    unsigned short pad3;
} POLY_GT4;
typedef struct { char *ordering[2]; char *other_buffers[6]; char *packets[2]; } RenderBufferPrefix;

#define setSemiTrans(p, abe) ((abe) ? ((p)->code |= 0x2) : ((p)->code &= ~0x2))

extern B4 D_800C22A0;
extern unsigned short D_800F3370;
extern unsigned short D_800F3374;
extern MATRIX *D_800BCFA4;
extern int D_8009CDD8;
extern int D_8009CDDC;
extern RenderBufferPrefix D_800B0E38;
extern int func_80077A64();
extern void func_80077BE4();
extern void func_80079754();
extern void func_800786E4();
extern void func_80077AC4();

void func_800D2370(SVECTOR *pos, SVECTOR *ang, int len, int width, int u, int v, int uw, int vh,
                   int clut, unsigned char *c0, unsigned char *c1, short bright, int abr)
{
    SVECTOR vt[4];
    B4 def;
    MATRIX m;
    int otz;
    int bias;
    POLY_GT4 *p;
    MATRIX *cam;

    def = D_800C22A0;
    p = (POLY_GT4 *)(D_800B0E38.packets[D_8009CDDC] + D_8009CDD8);
    cam = D_800BCFA4;
    bias = D_800F3374;
    D_8009CDD8 += sizeof(POLY_GT4);
    func_80077BE4(p);
    if (abr == 0xFF) {
        setSemiTrans(p, 0);
        p->tpage = D_800F3370;
    } else {
        setSemiTrans(p, 1);
        p->tpage = D_800F3370 | func_80077A64(0, abr, 0, 0);
    }
    p->clut = clut;
    gte_SetRotMatrix(cam);
    gte_SetTransMatrix(cam);
    gte_ldv0(pos);
    gte_rt();
    width /= 2;
    vt[0].vx = vt[2].vx = -width;
    vt[1].vx = vt[3].vx = width;
    vt[2].vz = vt[3].vz = len;
    vt[0].vy = vt[1].vy = vt[2].vy = vt[3].vy = 0;
    vt[0].vz = vt[1].vz = 0;
    gte_stlvnl(m.t);
    func_80079754(ang, &m);
    func_800786E4(&m);
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    gte_ldv3(&vt[0], &vt[1], &vt[2]);
    gte_rtpt();
    if (c0 == 0) {
        c0 = (unsigned char *)&def;
    }
    p->r0 = p->r1 = c0[0] * bright / 128;
    p->g0 = p->g1 = c0[1] * bright / 128;
    p->b0 = p->b1 = c0[2] * bright / 128;
    if (c1 == 0) {
        c1 = (unsigned char *)&def;
    }
    p->r2 = p->r3 = c1[0] * bright / 128;
    p->g2 = p->g3 = c1[1] * bright / 128;
    p->b2 = p->b3 = c1[2] * bright / 128;
    gte_stopz(&otz);
    if (otz == 0) {
        return;
    }
    gte_stsxy3(&p->x0, &p->x1, &p->x2);
    gte_avsz3();
    gte_stszotz(&otz);
    otz -= bias;
    if ((unsigned int)otz >= 0x1000) {
        return;
    }
    gte_ldv0(&vt[3]);
    gte_rtps();
    p->u0 = p->u1 = u;
    p->u2 = p->u3 = u + uw - 1;
    p->v0 = p->v2 = v;
    p->v1 = p->v3 = v + vh - 1;
    gte_stsxy(&p->x3);
    func_80077AC4((unsigned int *)D_800B0E38.ordering[D_8009CDDC] + otz, p);
}
