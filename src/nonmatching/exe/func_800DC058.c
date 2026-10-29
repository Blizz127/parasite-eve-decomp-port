/* func_800DC058 — EXE 0x800DC058, SLUS offset 0xCC858, 0x564 bytes.
 * NON_MATCHING (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback  int f(int mode, unsigned char *rec):
 *   mode 0: rec[0x10] = func_80071A54(); rec[8] = rec[0xC] = 0;
 *           func_800CE870(D_8009D254, 1, rec); hides both sprites (800C6D5C x2).
 *   mode 1: ++rec[0xC]; done (1) once the clock D_800E27EC reaches 0x40.
 *   mode 2: rec[0x10] += 0x10; rec[8] is a phase (0: ramp 0..0x18 frames,
 *           1: ramp 0..0xC frames, else full) giving the glow size (a0 / 4)
 *           and the second-sprite lead s4; draws the colour-curve sprite
 *           D_800F3418 at rec's vector and, when s4 != 0, a white twin D_800F342C.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { int x, y, z; } IntVector;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

extern unsigned char D_800E1E24[];
extern unsigned short D_800E11F6;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800E27EC;
extern unsigned short D_800F336C;
extern unsigned short D_800F336E;
extern unsigned short D_800F3370;
extern int D_800F3428;
extern int D_800F3418;
extern int D_800F342C;
extern int D_8009D254;
extern MATRIX *D_800BCFA4;

extern int func_80071A54();
extern void func_800CE870();
extern void func_800C6D5C();
extern void func_800CF3AC();
extern void func_80079754();
extern void func_80078CC4();
extern void func_800CEDA8();
extern int func_80077A64();
extern int func_80077AA4();
extern void func_800C6EC0(unsigned short a, unsigned short b);
extern void func_800C6ED8();
extern void func_800C6EF8();
extern void func_800C7098();
extern void func_800C71E4();
extern void func_800C6F4C();

int func_800DC058(int mode, unsigned char *rec)
{
    GteShortVector v;
    GteShortVector g;
    IntVector sc;
    unsigned char col[4];
    MATRIX m;
    int a0;
    int s4;
    int r;
    int s0;
    int a1;

    if (mode == 1) {
        W(rec, 0xC) = W(rec, 0xC) + 1;
        if (D_800E27EC < 0x40)
            return 0;
        return 1;
    }
    if (mode == 0) {
        W(rec, 0x10) = func_80071A54();
        W(rec, 8) = 0;
        W(rec, 0xC) = 0;
        func_800CE870(D_8009D254, 1, rec);
        func_800C6D5C(D_800F3418, 0, 0);
        func_800C6D5C(D_800F342C, 0, 0);
        return 0;
    }
    if (mode != 2)
        return 0;

    s4 = 0;
    W(rec, 0x10) = W(rec, 0x10) + 0x10;
    if (W(rec, 8) == 0) {
        a0 = (W(rec, 0xC) << 12) / 24;
        if (W(rec, 0xC) >= 0x18) {
            W(rec, 8) = 1;
            W(rec, 0xC) = 0;
        }
    } else if (W(rec, 8) == 1) {
        s4 = (((W(rec, 0xC) / 2) + 1) << 12) / 6;
        a0 = 0x1000;
        if (W(rec, 0xC) >= 0xC) {
            W(rec, 0xC) = 0;
            W(rec, 8) = mode;
        }
    } else {
        a0 = 0x1000;
    }

    sc.x = 0x44C;
    sc.z = 0x44C;
    sc.y = a0 / 4;
    v.x = 0;
    v.z = 0;
    v.y = (short)W(rec, 0x10);
    func_800CF3AC(D_800E1E24, col, D_800E27EC);
    func_80079754(&v, &m);
    func_80078CC4(&m, &sc);
    m.t[0] = SH(rec, 0);
    m.t[1] = SH(rec, 2);
    m.t[2] = SH(rec, 4);
    s0 = D_800E2850[D_800E11F6];
    D_800F336C = 1;
    D_800F3370 = s0;
    func_800CEDA8(1);
    D_800F336E = 1;
    gte_SetRotMatrix(D_800BCFA4);
    gte_SetTransMatrix(D_800BCFA4);
    r = func_80077A64(0, 3, 0, 0);
    s0 = (D_800E2850[D_800E11F6] | r) & 0xFFFF;
    a1 = D_800E1204[D_800F336C];
    if (D_800F336C == 4 && D_800F3428 != 0)
        a1 += 0xA;
    else
        a1 += 6;
    r = func_80077AA4(0, a1);
    func_800C6EC0(s0, r & 0xFFFF);
    func_800C6ED8(1);
    func_800C6EF8(D_800F3418);
    func_800C7098(D_800F3418, col[0], col[1], col[2]);
    func_800C71E4(D_800F3418, &m);

    sc.x = 0x4B0;
    sc.z = 0x4B0;
    v.x = 0x800;
    v.y = (short)W(rec, 0x10);
    v.z = 0;
    g.x = H(rec, 0);
    g.y = H(rec, 2) - 0x3E8;
    g.z = H(rec, 4);
    func_80079754(&v, &m);
    func_80078CC4(&m, &sc);
    m.t[0] = g.x;
    m.t[1] = g.y;
    m.t[2] = g.z;
    func_800C71E4(D_800F3418, &m);
    func_800C6F4C(D_800F3418);

    if (s4 == 0)
        return 0;

    sc.x = 0x4B0;
    sc.z = 0x4B0;
    v.x = 0;
    v.y = (short)(W(rec, 0x10) + s4);
    v.z = 0;
    g.x = H(rec, 0);
    g.y = H(rec, 2);
    g.z = H(rec, 4);
    func_80079754(&v, &m);
    func_80078CC4(&m, &sc);
    m.t[0] = g.x;
    m.t[1] = g.y;
    m.t[2] = g.z;
    r = func_80077A64(0, 1, 0, 0);
    s0 = (D_800E2850[D_800E11F6] | r) & 0xFFFF;
    a1 = D_800E1204[D_800F336C];
    if (D_800F336C == 4 && D_800F3428 != 0)
        a1 += 0xA;
    else
        a1 += 6;
    r = func_80077AA4(0, a1);
    func_800C6EC0(s0, r & 0xFFFF);
    func_800C6ED8(1);
    func_800C6EF8(D_800F342C);
    func_800C7098(D_800F342C, 0xFF, 0xFF, 0xFF);
    func_800C71E4(D_800F342C, &m);
    func_800C6F4C(D_800F342C);
    return 0;
}
