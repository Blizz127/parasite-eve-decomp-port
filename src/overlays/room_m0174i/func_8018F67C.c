/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_8018F67C — blob offset 0x694, 0x3fc bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0034i func_8018F434; C re-targeted by symbol address
 * (tools/analysis/room_lane/port.py). */

#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldclmv(r0) __asm__ volatile ( \
    "lhu $12,0(%0)\n\tlhu $13,6(%0)\n\tlhu $14,12(%0)\n\t" \
    "mtc2 $12,$9\n\tmtc2 $13,$10\n\tmtc2 $14,$11" : : "r"(r0) : "$12", "$13", "$14")
#define gte_rtir() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A49E012")
#define gte_stclmv(r0) __asm__ volatile ( \
    "mfc2 $12,$9\n\tmfc2 $13,$10\n\tmfc2 $14,$11\n\t" \
    "sh $12,0(%0)\n\tsh $13,6(%0)\n\tsh $14,12(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")
#define gte_ldlv0(r0) __asm__ volatile ( \
    "lhu $13,4(%0)\n\tlhu $12,0(%0)\n\tsll $13,$13,16\n\tor $12,$12,$13\n\t" \
    "mtc2 $12,$0\n\tlwc2 $1,8(%0)" : : "r"(r0) : "$12", "$13")
#define gte_rt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_stlvnl(r0) __asm__ volatile ( \
    "swc2 $9,0(%0)\n\tswc2 $10,4(%0)\n\tswc2 $11,8(%0)" : : "r"(r0) : "memory")
#define gte_MulMatrix0(r1, r2, r3) { \
    gte_SetRotMatrix(r1); \
    gte_ldclmv(r2); gte_rtir(); gte_stclmv(r3); \
    gte_ldclmv((char *)(r2) + 2); gte_rtir(); gte_stclmv((char *)(r3) + 2); \
    gte_ldclmv((char *)(r2) + 4); gte_rtir(); gte_stclmv((char *)(r3) + 4); }
#define gte_CompMatrix(r1, r2, r3) { \
    gte_MulMatrix0(r1, r2, r3); \
    gte_SetTransMatrix(r1); \
    gte_ldlv0((char *)(r2) + 20); gte_rt(); gte_stlvnl((char *)(r3) + 20); }

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

typedef struct {
    unsigned char f0, f1, f2, pad3;
    short f4, f6, f8, fA, fC, padE;
    short f10, f12, f14, pad16;
    int v18[3];
    MATRIX m;
} Obj;

extern SVECTOR D_8018EFF4;
extern MATRIX D_80196FC8;
extern int D_801974F4;
extern int D_801974FC;
extern unsigned short D_800942EC;
extern int *func_800C2B10(int);
extern int *func_800C2B28(int);
extern void func_80079754(SVECTOR *, MATRIX *);
extern void func_80078C34(MATRIX *, void *, void *);

void func_8018F67C(int a0, int a1, Obj *o)
{
    SVECTOR sv10;
    SVECTOR eff;
    MATRIX m20;
    SVECTOR sv40;
    MATRIX m48;
    int *p;

    eff = D_8018EFF4;
    sv40.vx = 0;
    p = func_800C2B10(4);
    sv40.vy = *p + *func_800C2B28(2);
    sv40.vz = 0;
    func_80079754(&sv40, &m48);
    sv10.vx = 0;
    sv10.vy = 0;
    sv10.vz = -*func_800C2B28(0);
    func_80078C34(&D_80196FC8, &sv10, o->v18);
    func_80078C34(&m48, o->v18, o->v18);
    o->m = D_80196FC8;
    SH(o, 0x8) = D_801974F4;
    SH(o, 0xA) = D_800942EC - 0x100;
    SH(o, 0xC) = D_801974FC;
    SH(o, 0x4) = 0x80;
    SH(o, 0x10) = 0;
    SH(o, 0x12) = 0;
    SH(o, 0x14) = 0;
    SH(o, 0x6) = 0;
    B(o, 0x2) = 0;
    B(o, 0x0) = 1;
    B(o, 0x1) = *func_800C2B10(3);
    func_80079754(&eff, &m20);
    gte_CompMatrix(&o->m, &m20, &o->m);
    gte_CompMatrix(&m48, &o->m, &o->m);
}
