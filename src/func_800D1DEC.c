/* func_800D1DEC — EXE, VRAM 0x800D1DEC, file offset 0xC25EC, 0x318 bytes.
 * Glow point: a TILE_1 core plus a quarter-brightness 3x3 TILE halo at the RTPS-projected position; optional DR_MODE semi-transparency.
 * Profile era_o2_g0.
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count separately
 * from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only).
 * Evidence: docs/evidence/func-800D1DEC/REPORT.md */
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
typedef struct { unsigned int tag; unsigned char r0, g0, b0, code; short x0, y0; } TILE_1;
typedef struct { unsigned int tag; unsigned char r0, g0, b0, code; short x0, y0; short w, h; } TILE;
typedef struct { unsigned int tag; unsigned int code[1]; } DR_MODE;
typedef struct { unsigned addr : 24; unsigned len : 8; } P_TAG;
typedef struct { char *ordering[2]; char *other_buffers[6]; char *packets[2]; } RenderBufferPrefix;

#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (unsigned int)(_addr))
#define getaddr(p) (unsigned int)(((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)
#define setSemiTrans(p, abe) ((abe) ? ((p)->code |= 0x2) : ((p)->code &= ~0x2))

extern unsigned short D_800F3374;
extern int D_8009CDD8;
extern int D_8009CDDC;
extern RenderBufferPrefix D_800B0E38;
extern int func_80077A64();
extern void func_80077C84();
extern void func_80077C24();
extern void func_80077B04();
extern void func_80077C44();
extern void func_80077AC4();

void func_800D1DEC(SVECTOR *pos, unsigned char *col, int bright, int abr)
{
    int otz;
    TILE_1 *p0;
    TILE *p1;
    DR_MODE *mode;
    unsigned int *ot;

    p0 = (TILE_1 *)(D_800B0E38.packets[D_8009CDDC] + D_8009CDD8);
    D_8009CDD8 += sizeof(TILE_1);
    p1 = (TILE *)(D_800B0E38.packets[D_8009CDDC] + D_8009CDD8);
    D_8009CDD8 += sizeof(TILE);
    gte_ldv0(pos);
    gte_rtps();
    func_80077C24(p0);
    func_80077B04(p0, 1);
    func_80077C44(p1);
    p1->r0 = p0->r0 = col[0] * bright / 128;
    p1->g0 = p0->g0 = col[1] * bright / 128;
    p1->b0 = p0->b0 = col[2] * bright / 128;
    p1->r0 >>= 2;
    p1->g0 >>= 2;
    p1->b0 >>= 2;
    gte_stsxy(&p0->x0);
    gte_stszotz(&otz);
    otz -= D_800F3374;
    if ((unsigned int)otz >= 0x1000) {
        return;
    }
    p1->x0 = p0->x0 - 1;
    p1->y0 = p0->y0 - 1;
    p1->w = p1->h = 3;
    func_80077AC4((unsigned int *)D_800B0E38.ordering[D_8009CDDC] + otz, p0);
    ot = (unsigned int *)((otz << 2) + (int)D_800B0E38.ordering[D_8009CDDC]);
    if (abr != 0xFF) {
        mode = (DR_MODE *)(D_800B0E38.packets[D_8009CDDC] + D_8009CDD8);
        D_8009CDD8 += sizeof(DR_MODE);
        func_80077C84(mode, 0, 1, func_80077A64(0, abr, 0, 0) & 0xFFFF);
        if (p1 != 0) {
            setSemiTrans(p1, 1);
            addPrim(ot, p1);
        }
        addPrim(ot, mode);
    } else if (p1 != 0) {
        addPrim(ot, p1);
    }
}
