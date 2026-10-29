/* func_800D6514 -- EXE, VRAM 0x800D6514, file offset 0xC6D14, 0x508 bytes.
 * NON_MATCHING behaviour C (proven by the e21 differential; not a byte match).
 * Three-mode effect callback (a0 = mode, a1 = effect record), the sibling of
 * func_800D5A00 (same descriptor shape, different effect):
 *   mode 0: seed the record (+8 = 0, +0/+2/+4 = halfwords 0x268/0x26A/0x26C of
 *           the object at D_800F32D0->+8, then +2 = D_800942EC), call
 *           func_800C6D5C(D_800F32D4, 0, 0xC0) and return
 *           func_800CE560(D_800F33E0->+8, 0x10, 0x18, func_800D629C).
 *   mode 1: on the first frame (D_800E27EC == 0) spawn 24 particles from
 *           func_800CE610(D_800F33E0->+8) (+8 = i*0xAA, +0xA = a1->+2,
 *           +0xC = rand); return 1 once D_800E27EC >= 0x18, else publish
 *           a1's position to D_800E21D8/DA/DC, D_800E21D4 = f / 3 with
 *           f = func_80077CF4((D_800E27EC << 10) / 24); return 0.
 *   mode 2: build a scaled rotation (func_80079754/func_80078CC4) from
 *           sin/cos of (D_800E27EC << 10) / 24, load the camera matrix
 *           *D_800BCFA4 into the GTE, draw the primitive twice (full scale
 *           then half scale) through func_800C6EC0..func_800C6F4C and reset
 *           the D_800F336x/337x draw parameters. Returns 0.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

extern int *D_800F32D0;
extern int D_800F32D4;
extern int *D_800F33E0;
extern unsigned short D_800942EC;
extern int D_800E27EC;
extern unsigned short D_800E11E4;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern short D_800E21D4;
extern unsigned short D_800E21D8;
extern unsigned short D_800E21DA;
extern unsigned short D_800E21DC;
extern short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern int D_800F3428;
extern MATRIX *D_800BCFA4;

extern unsigned char func_800D629C[];
extern int func_80071A54();
extern int func_80077A64();
extern int func_80077AA4();
extern int func_80077CF4();
extern int func_80077DC4();
extern void func_80078CC4();
extern void func_80079754();
extern void func_800C6D5C();
extern void func_800C6ED8();
extern void func_800C6EF8();
extern void func_800C6F4C();
extern void func_800C71E4();
extern int func_800CE560();
extern int func_800CE610();
extern void func_800CEDA8();
extern void func_800C6EC0(unsigned short a, unsigned short b);
extern void func_800C6FA0(int a, unsigned short b);

int func_800D6514(int mode, unsigned char *a1)
{
    /* retail keeps the matrix at sp+0x10, the scale vector at sp+0x30 and
     * the rotation angles at sp+0x40. */
    struct { MATRIX m; int v[3]; int pad; GteShortVector sv; } L;
    unsigned char *p;
    int i;
    int c;
    int s0;
    int s2;
    int x;
    int f;
    unsigned short a2;
    unsigned short v0;
    unsigned short flags;
    MATRIX *m;

    if (mode == 1) {
        if (D_800E27EC == 0) {
            for (i = 0; i < 0x18; i++) {
                p = (unsigned char *)func_800CE610(D_800F33E0[2]);
                if (p) {
                    SH(p, 0xA) = H(a1, 2);
                    SH(p, 0xC) = func_80071A54();
                    SH(p, 8) = i * 0xAA;
                }
            }
        }
        if (D_800E27EC >= 0x18)
            return 1;
        f = func_80077CF4((D_800E27EC << 10) / 24);
        D_800E21D8 = H(a1, 0);
        D_800E21DA = H(a1, 2);
        D_800E21DC = H(a1, 4);
        D_800E21D4 = f / 3;
        return 0;
    }
    if (mode < 2) {
        if (mode != 0)
            return 0;
        W(a1, 8) = 0;
        H(a1, 0) = H((unsigned char *)D_800F32D0[2], 0x268);
        H(a1, 2) = H((unsigned char *)D_800F32D0[2], 0x26A);
        H(a1, 4) = H((unsigned char *)D_800F32D0[2], 0x26C);
        H(a1, 2) = D_800942EC;
        func_800C6D5C(D_800F32D4, 0, 0xC0);
        return func_800CE560(D_800F33E0[2], 0x10, 0x18, func_800D629C);
    }
    if (mode != 2)
        return 0;
    f = func_80077CF4((D_800E27EC << 10) / 24);
    s0 = f / 2;
    c = func_80077DC4((D_800E27EC << 10) / 24);
    s2 = c / 32;
    L.v[2] = s0;
    L.v[1] = s0;
    L.v[0] = s0;
    L.sv.x = 0;
    L.sv.z = 0;
    L.sv.y = D_800E27EC << 5;
    func_80079754(&L.sv, &L.m);
    func_80078CC4(&L.m, L.v);
    L.m.t[0] = SH(a1, 0);
    L.m.t[1] = SH(a1, 2);
    L.m.t[2] = SH(a1, 4);
    D_800F336C = 0;
    D_800F3370 = D_800E2850[D_800E11E4];
    func_800CEDA8(0);
    D_800F336E = 0;
    m = D_800BCFA4;
    gte_SetRotMatrix(m);
    gte_SetTransMatrix(m);
    x = func_80077A64(0, 1, 0, 0);
    flags = D_800E2850[D_800E11E4] | x;
    a2 = D_800E1204[D_800F336C];
    if (D_800F336C == 4 && D_800F3428 != 0)
        a2 += 4;
    v0 = func_80077AA4(0xC0, a2);
    func_800C6EC0(flags, v0);
    func_800C6ED8(1);
    func_800C6EF8(D_800F32D4);
    func_800C6FA0(D_800F32D4, s2);
    func_800C71E4(D_800F32D4, &L.m);
    L.v[0] = L.v[0] / 2;
    L.v[1] = L.v[1] / 2;
    L.v[2] = L.v[2] / 2;
    func_80079754(&L.sv, &L.m);
    func_80078CC4(&L.m, L.v);
    L.m.t[0] = SH(a1, 0);
    L.m.t[1] = SH(a1, 2);
    L.m.t[2] = SH(a1, 4);
    func_800C71E4(D_800F32D4, &L.m);
    func_800C6F4C(D_800F32D4);
    D_800F3368 = 0x20;
    D_800F336A = 2;
    D_800F3376 = 0x20;
    D_800F3378 = 0x20;
    D_800F336C = 0;
    D_800F3370 = D_800E2850[D_800E11E4];
    func_800CEDA8(0);
    D_800F336E = 0;
    D_800F3372 = 0;
    D_800F3374 = 0x40;
    return 0;
}
