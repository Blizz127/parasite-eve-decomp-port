/* room_m0075i (PE.IMG room m0075i chunk 2, VRAM 0x8018EFE8)
 * func_8018F814 — blob offset 0x82C, 0x330 bytes. Profile era_o2_g0 (default).
 * Room light-model draw (func_800C2B50 render setup family, like room_m0421i
 * func_8018F980): an identity matrix scaled by +0x10 (func_80078CC4), composed
 * with the D_8018EFF4 rotation (inline Psy-Q gte_CompMatrix), then four
 * func_800C42A4 submissions at the owner's +0x134 / +0x1B4 points. The last two
 * use the floor height D_800942EC.
 * Twin (masked-body port): room_m0419i func_8018F858.
 * Levers: the identity matrix as chained assignments (retail stores m22, m11, m00,
 * then t2..t0, then the off-diagonals in reverse); the s field reads through SH
 * (a plain pointer, so each lh stays after the previous store); &D_800942EC in a
 * pointer local hp across the two calls ($s2).
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count separately
 * from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only). */
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
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { unsigned char pad[0xA]; short fA; } Lamp;

#define P(o, x) (*(void **)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

extern SVECTOR D_8018EFF4;
extern Lamp D_80194628;
extern unsigned char D_80194638[];
extern short D_800942EC;
extern unsigned char *func_800C2B50(void);
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800794C4();
extern void func_800C42A4();

void func_8018F814(int a0, int a1, unsigned char *p)
{
    MATRIX m;
    SVECTOR eff;
    MATRIX lw;
    VECTOR s2;
    VECTOR s;
    unsigned char *r;
    unsigned char *o;
    short *hp;

    r = func_800C2B50();
    eff = D_8018EFF4;
    o = P(P(r, 0), 0x238);
    func_800C2EAC(r[0x6C]);
    func_800C2FF0(0x20, 0x20);
    func_800C3098(0x10);
    func_800C3238(2);
    m.m[0][0] = m.m[1][1] = m.m[2][2] = 0x1000;
    m.t[0] = m.t[1] = m.t[2] = 0;
    m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
    func_80071A44(&s, 0, 16);
    s.vx = SH(p, 0x10);
    s.vy = SH(p, 0x10);
    s.vz = SH(p, 0x10);
    s2 = s;
    func_80078CC4(&m, &s2);
    func_800794C4(&eff, &lw);
    gte_CompMatrix(&m, &lw, &lw);
    D_80194628.fA = SH(p, 0x12);
    m.t[0] = W(o, 0x134);
    m.t[1] = W(o, 0x138);
    m.t[2] = W(o, 0x13C);
    func_800C42A4(&D_80194628, &m, 1);
    m.t[0] = W(o, 0x1B4);
    m.t[1] = W(o, 0x1B8);
    m.t[2] = W(o, 0x1BC);
    func_800C42A4(&D_80194628, &m, 1);
    hp = &D_800942EC;
    lw.t[0] = W(o, 0x134);
    lw.t[1] = *hp;
    lw.t[2] = W(o, 0x13C);
    func_800C42A4(D_80194638, &lw, 1);
    lw.t[0] = W(o, 0x1B4);
    lw.t[1] = *hp;
    lw.t[2] = W(o, 0x1BC);
    func_800C42A4(D_80194638, &lw, 1);
}
