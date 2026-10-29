/* room_m0429i — func_80196748, blob offset 0x7760, 0x584 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback, modes 1/2 on a record a1, read from the retail disassembly.
 * Mode 1 steps the record by state (+8): 0 moves it by (+0xC,+0xE,+0x18) for
 * an open-ended 0, 1 and 2 count +0xA up to 0x30 / 0x10 and return 1 when done.
 * Mode 2 loads the GTE rotation/translation from D_800BCFA4[0] and draws:
 * state 0 a glow (func_800D0E88) sized func_80077CF4(min(f,16) << 6) / 32;
 * state 1 a sprite at a rotated offset (func_800CFB7C, func_800D2370) timed
 * by f*1024/48; state 2 sets the D_800F3368 draw block and draws func_800CEE20.
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

extern B4 D_8018F224;
extern B4 D_8018F228;
extern B4 D_8018F22C;
extern GteShortVector D_8019B670;
extern unsigned char D_8019B3E4[];
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CFB7C(void *, short, void *);
extern void func_800CF3AC();
extern void func_800CEE20();
extern void func_800D0E88();
extern void func_800D2370();

int func_80196748(int mode, unsigned char *a1)
{
    struct {                    /* retail frame 0x38..0x60, 8-byte pitch */
        B4 col; int g0;
        B4 buf0; int g1;
        B4 buf1; int g2;
        B4 buf2; int g3;
        GteShortVector v;
    } L;
    int s0;
    int s2;
    int s3;
    int t;
    int f;
    int w;
    int clut;

    L.buf0 = D_8018F224;
    L.buf1 = D_8018F228;
    L.buf2 = D_8018F22C;
    switch (mode) {
    case 1:
        switch (SH(a1, 8)) {
        case 0:
            H(a1, 0xA)++;
            H(a1, 0) += H(a1, 0xC);
            H(a1, 2) += H(a1, 0xE);
            H(a1, 4) += 0x18;
            return 0;
        case 1:
            H(a1, 0xA)++;
            H(a1, 0) += 8;
            H(a1, 2) += H(a1, 0xE);
            H(a1, 4) += 0x18;
            if (SH(a1, 0xA) < 0x30)
                return 0;
            return 1;
        case 2:
            H(a1, 0xA)++;
            H(a1, 2) += 4;
            H(a1, 0) += 4;
            H(a1, 4) += 0x30;
            if (SH(a1, 0xA) < 0x10)
                return 0;
            return 1;
        }
        return 0;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        switch (SH(a1, 8)) {
        case 0:
            s0 = SH(a1, 0xA);
            if (s0 >= 0x10)
                s0 = 0x10;
            s2 = func_80077CF4(s0 << 6) / 32;
            if (H(a1, 0xA) & 1)
                s3 = 0x5C;
            else
                s3 = 0x80;
            func_800CF3AC(D_8019B3E4, &L.col, s0);
            func_800D0E88(&D_8019B670, a1, 0x9C4, s2, &L.buf0, 0, 0, (short)s3, 1);
            return 0;
        case 1:
            f = SH(a1, 0xA);
            s0 = func_80077DC4((f << 10) / 48);
            f = SH(a1, 0xA);
            s3 = (f << 7) / 48;
            t = func_80077CF4((f << 10) / 48);
            s2 = t / 4 + 0x1F4;
            func_800CFB7C(a1, SH(a1, 0xC) * s0 / 4096, &L.v);
            L.v.x += D_8019B670.x;
            L.v.y += D_8019B670.y;
            L.v.z += D_8019B670.z;
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0x70, w);
            func_800D2370(&L.v, a1, s2, 0x8C, 0x80, 0x80, 0x80, 0x10,
                          clut & 0xFFFF, &L.buf1, &L.buf2, (short)s3, 1);
            return 0;
        case 2:
            s2 = func_80077DC4(SH(a1, 0xA) << 6) * 2;
            func_800CF3AC(D_8019B3E4, &L.col, SH(a1, 0xA));
            D_800F3368.a68 = 0x40;
            D_800F3368.a6A = 4;
            D_800F3368.a76 = 0x40;
            D_800F3368.a78 = 0x40;
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 0;
            D_800F3368.a70 = D_800E2850[D_800E11EA];
            SH(a1, 6) = 1;
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0, w);
            func_800CEE20(&D_8019B670, a1, s2, s2, 0x40, clut & 0xFFFF, 1, 0x80, &L.col);
            return 0;
        }
        return 0;
    }
    return 0;
}
