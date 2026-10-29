/* room_m0318i — func_8019326C, blob offset 0x4284, 0x3CC bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback, a0 = mode, a1 = effect record (+0/+2/+4 position, +6 phase
 * flag, +8 / +0xA / +0xE velocity-like shorts, +0xC frame counter).
 *   mode 1, +6 == 0: seed +0/+2/+4 from D_80199904/06/08; +0xA += 0x20 +
 *           (f << 2); +8 = sin((f << 11) / 24) / 32; +0xE = (f << 12) / 24
 *           (f = D_800E27EC). On odd f spawn a particle via func_800CE610
 *           (D_800F33E0->+8): +6 = 1, copy +0/+2/+4/+8/+0xE. Returns 1 once
 *           f >= 0x18, else bumps +0xC and returns 0.
 *   mode 1, +6 != 0: +0xA += 3, +8 = +8 * 3 / 4; returns 1 once f >= 4, else
 *           bumps +0xC and returns 0.
 *   mode 2: RotMatrix {0, +0xA, 0} -> stack matrix at sp+0x18, ScaleMatrix by
 *           {0x400, (+0xE) / 4, 0x400}, translation = a1 +0/+2/+4 (ints), set
 *           the D_800F336C draw block (3, 1, D_800E2850[D_800E11FA]), load the
 *           D_800BCFA4[0] matrix into the GTE, then draw with the colour
 *           func_80077A64 | table word, CLUT func_80077AA4 and a size pair
 *           derived from +8 (/3 and /2). Returns 0.
 *   other : returns 0.
 * The stack vector at sp+0x38 is {0x400, +0xE / 4, 0x400}; word 0x44 is never
 * written (matches retail).
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

extern unsigned short D_80199904;
extern unsigned short D_80199906;
extern unsigned short D_80199908;
extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern unsigned short D_800F336C;
extern unsigned short D_800F336E;
extern unsigned short D_800F3370;
extern int D_800F3428;
extern int D_800F32D8;
extern MATRIX *D_800BCFA4[];

extern int func_80077CF4();
extern int func_80077A64();
extern int func_80077AA4();
extern int func_80079754();
extern int func_80078CC4();
extern unsigned char *func_800CE610();
extern int func_800C6EC0();
extern int func_800C6ED8();
extern int func_800C6EF8();
extern int func_800C7098();
extern int func_800C71E4();
extern int func_800C6F4C();

int func_8019326C(int mode, unsigned char *a1)
{
    GteShortVector rot;
    MATRIX m;
    int scale[4];
    unsigned char *p;
    int s;
    int v;
    int w;
    int a;

    if (mode == 1) {
        if (SH(a1, 6) == 0) {
            H(a1, 0) = D_80199904;
            H(a1, 4) = D_80199908;
            H(a1, 0xA) = H(a1, 0xA) + 0x20 + (D_800E27EC << 2);
            H(a1, 2) = D_80199906;
            v = func_80077CF4((D_800E27EC << 11) / 24);
            SH(a1, 8) = v / 32;
            SH(a1, 0xE) = (D_800E27EC << 12) / 24;
            if (D_800E27EC & 1) {
                p = func_800CE610(*(int *)(D_800F33E0 + 8));
                if (p != 0) {
                    H(p, 6) = 1;
                    H(p, 0) = H(a1, 0);
                    H(p, 2) = H(a1, 2);
                    H(p, 4) = H(a1, 4);
                    H(p, 8) = H(a1, 8);
                    H(p, 0xE) = H(a1, 0xE);
                }
            }
            if (!(D_800E27EC < 0x18))
                return 1;
        } else {
            a = SH(a1, 8);
            H(a1, 0xA) += 3;
            SH(a1, 8) = (a * 3) / 4;
            if (!(D_800E27EC < 4))
                return 1;
        }
        H(a1, 0xC)++;
        return 0;
    }
    if (mode == 2) {
        scale[2] = 0x400;
        scale[0] = 0x400;
        scale[1] = SH(a1, 0xE) / 4;
        rot.x = 0;
        rot.y = H(a1, 0xA);
        rot.z = 0;
        func_80079754(&rot, &m);
        func_80078CC4(&m, scale);
        m.t[0] = SH(a1, 0);
        m.t[1] = SH(a1, 2);
        m.t[2] = SH(a1, 4);
        D_800F336C = 3;
        D_800F336E = 1;
        D_800F3370 = D_800E2850[D_800E11FA];
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        v = func_80077A64(0, 1, 0, 0);
        s = (unsigned short)(D_800E2850[D_800E11FA] | v);
        w = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0)
            w += 8;
        else
            w += 4;
        v = func_80077AA4(0, w);
        func_800C6EC0(s, v & 0xFFFF);
        func_800C6ED8(1);
        func_800C6EF8(D_800F32D8);
        a = SH(a1, 8);
        func_800C7098(D_800F32D8, (a / 3) & 0xFF, 0, (a / 2) & 0xFF);
        func_800C71E4(D_800F32D8, &m);
        func_800C6F4C(D_800F32D8);
        return 0;
    }
    return 0;
}
