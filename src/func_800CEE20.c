/* func_800CEE20 — EXE, VRAM 0x800CEE20, file offset 0xBF620, 0x58C bytes.
 * Effect sprite model: one POLY_FT4 template (tpage/abr, clut, brightness-scaled colour, texture-cell UVs), camera-space position, RotMatrix(angles) scaled by the cell size; each of the D_800E1210[type] model quads is RTPT/RTPS-projected and linked at OT[otz - bias].
 * Profile era_o2_g0.
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count separately
 * from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only).
 * Evidence: docs/evidence/func-800CEE20/REPORT.md */
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
typedef struct { unsigned char b[8]; } B8;
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
typedef struct { unsigned addr : 24; unsigned len : 8; } P_TAG;
typedef struct { char *ordering[2]; char *other_buffers[6]; char *packets[2]; } RenderBufferPrefix;

#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (unsigned int)(_addr))
#define getaddr(p) (unsigned int)(((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)
#define setlen(p, _len) (((P_TAG *)(p))->len = (unsigned char)(_len))
#define setPolyFT4(p) setlen(p, 9), (p)->code = 0x2C
#define setUV4(p, _u0, _v0, _u1, _v1, _u2, _v2, _u3, _v3) \
    (p)->u0 = (_u0), (p)->v0 = (_v0), (p)->u1 = (_u1), (p)->v1 = (_v1), \
    (p)->u2 = (_u2), (p)->v2 = (_v2), (p)->u3 = (_u3), (p)->v3 = (_v3)
#define setUVWH(p, _u0, _v0, _w, _h) \
    (p)->u0 = (_u0), (p)->v0 = (_v0), (p)->u1 = (_u0) + (_w), (p)->v1 = (_v0), \
    (p)->u2 = (_u0), (p)->v2 = (_v0) + (_h), (p)->u3 = (_u0) + (_w), (p)->v3 = (_v0) + (_h)

extern B8 D_800C2268;
extern unsigned short D_800F3370;
extern unsigned short D_800F336E;
extern unsigned short D_800F336C;
extern int D_800F3428;
extern unsigned short D_800F3376;
extern unsigned short D_800F3378;
extern unsigned short D_800F3372;
extern unsigned short D_800F3374;
extern MATRIX *D_800BCFA4;
extern int D_8009CDD8;
extern int D_8009CDDC;
extern RenderBufferPrefix D_800B0E38;
extern unsigned short D_800E1210[];
extern SVECTOR *D_800E13BC[];
extern int func_80077A64();
extern void func_80079754();
extern void func_80078CC4();
extern void func_800786E4();

void func_800CEE20(SVECTOR *pos, SVECTOR *ang, int sx, int sy, int tex, int clut, int abr,
                   int bright, unsigned char *col)
{
    POLY_FT4 pk;
    VECTOR scale;
    B8 def;
    MATRIX m;
    int otz;
    POLY_FT4 *p;
    SVECTOR *vtx;
    int u, v, w, h;
    int bias;
    int i;
    MATRIX *cam;

    def = D_800C2268;
    if (ang == 0) {
        ang = (SVECTOR *)&def;
    }
    setPolyFT4(&pk);
    if (abr == 0xFF) {
        pk.code = 0x2C;
        pk.tpage = D_800F3370;
    } else {
        pk.code = 0x2E;
        pk.tpage = D_800F3370 | func_80077A64(0, abr, 0, 0);
    }
    cam = D_800BCFA4;
    pk.clut = clut;
    if (col == 0) {
        w = v = h = bright;
    } else {
        w = bright * col[0] / 128;
        h = bright * col[1] / 128;
        v = bright * col[2] / 128;
    }
    pk.r0 = w;
    pk.g0 = h;
    pk.b0 = v;
    if (D_800F336E != 0) {
        u = tex & 0xF;
        v = 0;
        if (u >= 8) {
            u -= 8;
            v = 0x20;
        }
        u <<= 4;
        v += (tex / 16) << 4;
    } else {
        u = (tex & 0xF) << 4;
        v = (tex / 16) << 4;
    }
    if (D_800F336C == 4 && D_800F3428 != 0) {
        v += 0x60;
    }
    w = D_800F3376;
    h = D_800F3378;
    pk.u0 = pk.u2 = u;
    pk.v1 = pk.v0 = v;
    pk.u1 = pk.u3 = u + w - 1;
    pk.v2 = pk.v3 = v + h - 1;
    scale.vx = sx * (w >> 4);
    scale.vy = sy * (h >> 4);
    scale.vz = 0x1000;
    gte_SetRotMatrix(cam);
    gte_SetTransMatrix(cam);
    gte_ldv0(pos);
    gte_rt();
    p = (POLY_FT4 *)(D_800B0E38.packets[D_8009CDDC] + D_8009CDD8);
    D_8009CDD8 += D_800E1210[D_800F3372] * sizeof(POLY_FT4);
    bias = D_800F3374;
    func_80079754(ang, &m);
    func_80078CC4(&m, &scale);
    gte_stlvnl(m.t);
    if (ang->pad == 1) {
        func_800786E4(&m);
    }
    gte_SetRotMatrix(&m);
    gte_SetTransMatrix(&m);
    vtx = D_800E13BC[D_800F3372];
    for (i = 0; i < D_800E1210[D_800F3372]; i++, p++, vtx += 4) {
        gte_ldv3(&vtx[0], &vtx[1], &vtx[2]);
        gte_rtpt();
        *p = pk;
        gte_stsxy3(&p->x0, &p->x1, &p->x2);
        gte_stopz(&otz);
        if (otz == 0) {
            return;
        }
        gte_stszotz(&otz);
        gte_ldv0(&vtx[3]);
        gte_rtps();
        otz -= bias;
        if ((unsigned int)otz < 0x1000) {
            gte_stsxy(&p->x3);
            addPrim((unsigned int *)D_800B0E38.ordering[D_8009CDDC] + otz, p);
        }
    }
}
