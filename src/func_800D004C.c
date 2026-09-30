/* func_800D004C — EXE, VRAM 0x800D004C, file offset 0xC084C, 0x6DC bytes.
 * Shaded fan: n POLY_G3 triangles around the centre with alternating radii, optional per-segment DR_MODE for semi-transparency.
 * Profile era_o2_g0_expand_div.
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count separately
 * from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only).
 * Evidence: docs/evidence/func-800D004C/REPORT.md */
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

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
typedef struct { unsigned char b[8]; } B8;
typedef struct {
    unsigned int tag;
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
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char r1, g1, b1, pad1;
    short x1, y1;
    unsigned char r2, g2, b2, pad2;
    short x2, y2;
} POLY_G3;
typedef struct { unsigned int tag; unsigned int code[1]; } DR_MODE;
typedef struct { unsigned addr : 24; unsigned len : 8; } P_TAG;

#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (unsigned int)(_addr))
#define getaddr(p) (unsigned int)(((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)
#define setSemiTrans(p, abe) ((abe) ? ((p)->code |= 0x2) : ((p)->code &= ~0x2))

extern B8 D_800C2260;
extern VECTOR D_800C2290;
extern unsigned short D_800F3374;
extern MATRIX *D_800BCFA4;
extern int D_8009CDD8;
extern int D_8009CDDC;
typedef struct { char *ordering[2]; char *other_buffers[6]; char *packets[2]; } RenderBufferPrefix;
extern RenderBufferPrefix D_800B0E38;
extern int func_80077A64();
extern void func_80077C84();
extern void func_80077B84();
extern void func_80079754();
extern void func_80078CC4();
extern void func_800786E4();
extern int func_80077DC4();
extern int func_80077CF4();

void func_800D004C(SVECTOR *pos, int r0, int r1, int n, SVECTOR *ang, int sx, int sy,
                   unsigned char *c0, unsigned char *c1, int bright, int abr)
{
    POLY_G3 pk;
    short radii[2];
    SVECTOR v[3];
    SVECTOR sv;
    B8 def;
    VECTOR scale;
    CVECTOR col0;
    CVECTOR col1;
    MATRIX m;
    int otz;
    int bias;
    MATRIX *cam;
    POLY_G3 *p;
    DR_MODE *mode;
    unsigned int *ot;
    int i;
    int a0, a1;

    def = D_800C2260;
    scale = D_800C2290;
    if (n < 4) {
        return;
    }
    bias = D_800F3374;
    cam = D_800BCFA4;
    if (c0 == 0) {
        col0.r = col0.g = col0.b = 0;
    } else {
        col0.r = c0[0] * bright / 128;
        col0.g = c0[1] * bright / 128;
        col0.b = c0[2] * bright / 128;
    }
    if (c1 == 0) {
        col1.r = col1.g = col1.b = 0;
    } else {
        col1.r = c1[0] * bright / 128;
        col1.g = c1[1] * bright / 128;
        col1.b = c1[2] * bright / 128;
    }
    gte_SetRotMatrix(cam);
    gte_SetTransMatrix(cam);
    sv.vx = pos->vx;
    sv.vy = pos->vy;
    sv.vz = pos->vz;
    gte_ldv0(&sv);
    gte_rt();
    if (ang == 0) {
        ang = (SVECTOR *)&def;
    }
    scale.vx = sx;
    scale.vy = sy;
    gte_stlvnl(m.t);
    func_80079754(ang, &m);
    func_80078CC4(&m, &scale);
    if (ang->pad != 0) {
        func_800786E4(&m);
    }
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    func_80077B84(&pk);
    pk.r0 = col0.r;
    pk.g0 = col0.g;
    pk.b0 = col0.b;
    pk.r1 = col1.r;
    pk.g1 = col1.g;
    pk.b1 = col1.b;
    pk.r2 = col1.r;
    pk.g2 = col1.g;
    pk.b2 = col1.b;
    p = (POLY_G3 *)(D_800B0E38.packets[D_8009CDDC] + D_8009CDD8);
    D_8009CDD8 += n * sizeof(POLY_G3);
    v[0].vx = v[0].vy = v[0].vz = v[1].vz = v[2].vz = 0;
    radii[0] = r0;
    radii[1] = r1;
    for (i = 0; i < n; i++, p++) {
        a0 = (i << 12) / n;
        a1 = ((i + 1) << 12) / n;
        v[1].vx = func_80077DC4(a0) * radii[i & 1] / 4096;
        v[1].vy = func_80077CF4(a0) * radii[i & 1] / 4096;
        v[2].vx = func_80077DC4(a1) * radii[(i + 1) & 1] / 4096;
        v[2].vy = func_80077CF4(a1) * radii[(i + 1) & 1] / 4096;
        gte_ldv3(&v[0], &v[1], &v[2]);
        gte_rtpt();
        *p = pk;
        gte_stsxy3(&p->x0, &p->x1, &p->x2);
        gte_avsz3();
        gte_stszotz(&otz);
        otz -= bias;
        if ((unsigned int)otz >= 0x1000) {
            return;
        }
        ot = (unsigned int *)((otz << 2) + (int)D_800B0E38.ordering[D_8009CDDC]);
        if (abr != 0xFF) {
            mode = (DR_MODE *)(D_800B0E38.packets[D_8009CDDC] + D_8009CDD8);
            D_8009CDD8 += sizeof(DR_MODE);
            func_80077C84(mode, 0, 1, func_80077A64(0, abr, 0, 0) & 0xFFFF);
            if (p != 0) {
                setSemiTrans(p, 1);
                addPrim(ot, p);
            }
            addPrim(ot, mode);
        } else if (p != 0) {
            addPrim(ot, p);
        }
    }
}
