/* room_m0432i — func_8019702C, blob offset 0x8044, 0x510 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback, read from the retail disassembly; same family as
 * func_801962FC / func_801957CC (frame counter at +0x12, kind at +0x10).
 * cmd 1 (step): kind 0 grows +0xE by 0x20 per frame (done at 0x20 frames);
 *   kind 1 waits for D_80199504 != 0 (frame counter still counts);
 *   kind 2 moves +8 by 8 and +0xA by 0xC per frame (done at 0x10 frames).
 * cmd 2 (draw): kind 0 places the sprite from func_800CF844 around a vector
 *   scaled by the sine of frame << 5, seeds the D_800F3368 draw block and
 *   draws with func_800CEE20 (size 1.5 * cosine, colour via func_800CF3AC);
 *   kind 1 loads the camera rotation/translation into the GTE and draws a
 *   0xA0-wide sprite through func_800D2370 (colour byte 0x96 on even frames,
 *   0x80 on odd frames, x origin 0x80 or 0xC0 by bit 0 of +0xA);
 *   kind 2 loads the camera matrix and draws a ring through func_800D0E88
 *   sized cosine(frame << 6) / 8.
 * The 8-byte seed copied from D_8018F1F4 has its halfword at +4 overwritten
 * with +0xE - frame * 30 (kind 0). The two 4-byte colour blocks D_8018F21C /
 * D_8018F220 are copied to the stack on entry and passed to func_800D0E88.
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
extern B4 D_8018F21C;
extern B4 D_8018F220;
extern unsigned char D_8019946C[];
extern short D_80199504;
extern MATRIX *D_800BCFA4[];
extern unsigned short D_800E11EA;
extern unsigned short D_800E11FA;
extern unsigned short D_800E120A;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern int D_800F3428;
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CF844();
extern void func_800CF3AC();
extern void func_800CEE20();
extern void func_800D2370();
extern void func_800D0E88();

int func_8019702C(int mode, unsigned char *a1)
{
    GteShortVector seed;
    GteShortVector v;
    struct { B4 c1; B4 gap; B4 c2; } cc;   /* retail: c1 at sp+0x50, c2 at sp+0x58 */
    B4 col;
    int s0;
    int r;
    int w;
    int clut;
    int s4;

    seed = D_8018F1F4;
    cc.c1 = D_8018F21C;
    cc.c2 = D_8018F220;
    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0:
            H(a1, 0x12)++;
            H(a1, 0xE) += 0x20;
            if (SH(a1, 0x12) < 0x20)
                return 0;
            return 1;
        case 1:
            H(a1, 0x12)++;
            if (D_80199504 != 0)
                return 1;
            return 0;
        case 2:
            H(a1, 0x12)++;
            H(a1, 8) += 8;
            H(a1, 0xA) += 0xC;
            if (SH(a1, 0x12) < 0x10)
                return 0;
            return 1;
        }
        return 0;
    case 2:
        switch (SH(a1, 0x10)) {
        case 0:
            s0 = SH(a1, 0x12) << 5;
            r = func_80077CF4(s0);
            func_800CF844(a1, &v, 0xC8, a1 + 8, r * SH(a1, 6) / 4096, SH(a1, 0xE));
            s0 = func_80077DC4(s0) * 3 / 2;
            D_800F3368.a68 = 0x20;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            D_800F3368.a6A = 2;
            seed.z = H(a1, 0xE) - SH(a1, 0x12) * 30;
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 1;
            D_800F3368.a70 = D_800E2850[D_800E11FA];
            func_800CF3AC(D_8019946C, &col, SH(a1, 0x12));
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 0xA;
            else
                w += 6;
            clut = func_80077AA4(0, w);
            func_800CEE20(&v, &seed, s0, s0, D_800F3368.a6A + 0x64, clut & 0xFFFF, 1, 0x80, &col);
            return 0;
        case 1:
            if (H(a1, 0x12) & 1)
                s4 = 0x80;
            else
                s4 = 0x96;
            *(int *)&col = 0x808080;
            s0 = ((H(a1, 0xA) & 1) << 6) + 0x80;
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 0;
            D_800F3368.a70 = D_800E2850[D_800E11EA];
            clut = func_80077AA4(0x20, D_800E120A);
            func_800D2370(a1, a1 + 8, SH(a1, 6), 0xBE, s0, 0xA0, 0x40, 0x20,
                          clut & 0xFFFF, &col, 0, s4, 1);
            return 0;
        case 2:
            r = func_80077DC4(SH(a1, 0x12) << 6);
            s0 = r / 8;
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            func_800D0E88(a1, a1 + 8, 0x1004, s0, &cc.c1, &cc.c2, &cc.c2, 0x80, 1);
            return 0;
        }
        return 0;
    }
    return 0;
}
