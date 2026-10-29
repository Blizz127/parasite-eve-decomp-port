/* room_m0432i — func_80193940, blob offset 0x4958, 0x74C bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback, read from the retail disassembly; sibling of
 * func_80192548 / func_8019485C (frame counter at +0x12, kind at +0x10).
 * cmd 1 (step): position (+0/+2/+4) += velocity (+8/+0xA/+0xC) and the frame
 *   counter ticks; kind 0 damps +8/+0xC by 31/32 and bounces +0xA off the
 *   floor D_800942EC (done at 0x10 frames); kind 1 damps by 127/128, +0xA also
 *   loses 1 per frame, same bounce (done at 0x18 frames); kind 2 only ticks
 *   the frame (done at 0x18 frames).
 * cmd 2 (draw): kind 0 pulses a sprite through func_800CEE20 (size 1.5 * sine
 *   of frame << 7, rotation -(frame << 5), colour via func_800CF3AC); kind 1
 *   spins a sprite of size sine/2 (rotation frame << 5); kind 2 loads the
 *   camera matrix/translation into the GTE and draws a ring-like sprite from
 *   the second seed vector (z = D_800E27EC << 6), size 2 * (sine/2 + 0x800).
 * The three seeds copied on entry are D_8018F1F4 / D_8018F1FC (8-byte vectors)
 * and D_8018F204 (4-byte colour block, rewritten by func_800CF3AC).
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char c[4]; } B4;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern GteShortVector D_8018F1F4;
extern GteShortVector D_8018F1FC;
extern B4 D_8018F204;
extern unsigned char D_80199308[];
extern short D_800942EC;
extern MATRIX *D_800BCFA4[];
extern unsigned short D_800E11EA;
extern unsigned short D_800E11FA;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern int D_800E27EC;
extern int D_800F3428;
extern short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3376;
extern short D_800F3378;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CF3AC();
extern void func_800CEE20();

int func_80193940(int mode, unsigned char *a1)
{
    GteShortVector va;
    GteShortVector vb;
    B4 col;
    int s0;
    int s1;
    int s2;
    int w;
    int clut;

    va = D_8018F1F4;
    vb = D_8018F1FC;
    col = D_8018F204;
    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0:
            H(a1, 0) += H(a1, 8);
            H(a1, 2) += H(a1, 0xA);
            H(a1, 0x12)++;
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 31 / 32;
            SH(a1, 0xC) = SH(a1, 0xC) * 31 / 32;
            if (SH(a1, 2) >= D_800942EC)
                H(a1, 0xA) = -SH(a1, 0xA);
            if (SH(a1, 0x12) < 0x10)
                return 0;
            return 1;
        case 1:
            H(a1, 0) += H(a1, 8);
            H(a1, 2) += H(a1, 0xA);
            H(a1, 0x12)++;
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 127 / 128;
            SH(a1, 0xC) = SH(a1, 0xC) * 127 / 128;
            H(a1, 0xA) = H(a1, 0xA) - 1;
            if (SH(a1, 2) >= D_800942EC)
                H(a1, 0xA) = -SH(a1, 0xA);
            if (SH(a1, 0x12) < 0x18)
                return 0;
            return 1;
        case 2:
            H(a1, 0x12)++;
            if (SH(a1, 0x12) < 0x18)
                return 0;
            return 1;
        }
        return 0;
    case 2:
        switch (SH(a1, 0x10)) {
        case 0:
            s2 = func_80077CF4(SH(a1, 0x12) << 7) * 3 / 2;
            s0 = func_80077CF4(SH(a1, 0x12) << 7);
            D_800F336A = 2;
            D_800F3368 = 0x20;
            D_800F3376 = 0x20;
            D_800F3378 = 0x20;
            va.z = -(SH(a1, 0x12) << 5);
            D_800F336C = 3;
            D_800F336E = 0;
            D_800F3370 = D_800E2850[D_800E11EA];
            s1 = s0 / 160;
            func_800CF3AC(D_80199308, &col, SH(a1, 0x12));
            w = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0, w);
            func_800CEE20(a1, &va, s2, s2, D_800F336A * (SH(a1, 0x12) / 2) + 0x80,
                          clut & 0xFFFF, 2, s1, 0);
            return 0;
        case 1:
            s2 = func_80077DC4((SH(a1, 0x12) << 10) / 24);
            D_800F336A = 2;
            D_800F3368 = 0x20;
            D_800F3376 = 0x20;
            D_800F3378 = 0x20;
            va.z = SH(a1, 0x12) << 5;
            D_800F336C = 3;
            D_800F336E = 1;
            D_800F3370 = D_800E2850[D_800E11FA];
            func_800CF3AC(D_80199308, &col, SH(a1, 0x12));
            w = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0)
                w += 0xA;
            else
                w += 6;
            clut = func_80077AA4(0, w);
            func_800CEE20(a1, &va, s2 / 2, s2 / 2, 0x44, clut & 0xFFFF, 1, 0x80, &col);
            return 0;
        case 2:
            s1 = func_80077CF4((SH(a1, 0x12) << 11) / 24) / 32;
            s2 = func_80077DC4((SH(a1, 0x12) << 10) / 24) / 2 + 0x800;
            vb.z = D_800E27EC << 6;
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            D_800F3368 = 0x40;
            D_800F336A = 4;
            D_800F3376 = 0x40;
            D_800F3378 = 0x40;
            D_800F336C = 3;
            D_800F336E = 1;
            D_800F3370 = D_800E2850[D_800E11FA];
            func_800CF3AC(D_80199308, &col, SH(a1, 0x12));
            w = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0)
                w += 6;
            else
                w += 2;
            clut = func_80077AA4(0, w);
            func_800CEE20(a1, &vb, s2 * 2, s2 * 2, 0x40, clut & 0xFFFF, 1, s1 / 2, &col);
            return 0;
        }
        return 0;
    }
    return 0;
}
