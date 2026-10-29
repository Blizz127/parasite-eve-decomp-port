/* room_m0318i — func_8019646C, blob offset 0x7484, 0x3B4 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback: a0 = command (1 update, 2 draw), a1 = effect record
 * (+0/+2/+4 position, +6, +8/+0xA/+0xC vector, +0x10 state, +0x12 frame).
 * cmd 1 state 0 waits 32 frames; state 1 integrates +8/+A/+C += +0/+2/+4 for
 * 40 frames. cmd 2 state 0 draws with func_800D0728 (colour from D_80199798,
 * scale sin(f << 5)); state 1 draws the sprite with func_800D2370, size
 * sin(f << 5) * 0x76C >> 12 (f <= 0x20), height cos(f*1024/40) / 64 + 0x20,
 * CLUT from D_800E1204[D_800F336C] (+4 when D_800F336C == 4 and D_800F3428).
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

extern B4 D_8018F1FC;
extern unsigned char D_80199798[];
extern unsigned short D_80199930;
extern unsigned short D_80199932;
extern unsigned short D_80199934;
extern unsigned short D_800E1204[];
extern unsigned short D_800F336C;
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CF3AC();
extern void func_800D0728();
extern void func_800D2370();

int func_8019646C(int mode, unsigned char *a1)
{
    GteShortVector v;
    B4 col[2];
    B4 buf;
    int s0;
    int s2;
    int s4;
    int s5;
    int s6;
    int w;
    int clut;
    short e;

    buf = D_8018F1FC;
    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0:
            H(a1, 0x12) = H(a1, 0x12) + 1;
            if (SH(a1, 0x12) < 0x20)
                return 0;
            return 1;
        case 1:
            H(a1, 0x12) = H(a1, 0x12) + 1;
            H(a1, 0xA) = H(a1, 0xA) + H(a1, 2);
            H(a1, 8) = H(a1, 8) + H(a1, 0);
            H(a1, 0xC) = H(a1, 0xC) + H(a1, 4);
            if (SH(a1, 0x12) < 0x28)
                return 0;
            return 1;
        }
        return 0;
    case 2:
        switch (SH(a1, 0x10)) {
        case 0:
            s2 = func_80077CF4(SH(a1, 0x12) << 5);
            func_800CF3AC(D_80199798, col, SH(a1, 0x12));
            func_800D0728(a1, 0x578, 0x898, 0xC, a1 + 8, s2, s2, 0, col, 0x80, 1);
            return 0;
        case 1:
            s0 = (SH(a1, 0x12) << 10) / 40;
            s4 = func_80077DC4(s0) / 32;
            if (H(a1, 0x12) & 1)
                s4 = (s4 * 2) / 3;
            *(unsigned short *)&v = D_80199930;
            *((unsigned short *)&v + 1) = D_80199932;
            *((unsigned short *)&v + 2) = D_80199934;
            s5 = 0x76C;
            if (SH(a1, 0x12) < 0x21)
                s5 = func_80077CF4(SH(a1, 0x12) << 5) * 0x76C / 4096;
            s6 = func_80077DC4(s0) / 64 + 0x20;
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            e = H(a1, 0x12) & 3;
            w = D_800E1204[D_800F336C];
            if (D_800F336C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0x50, w);
            func_800D2370(&v, a1 + 8, s5, s6, (e & 1) << 7, ((unsigned)e >> 1) * 16 + 0x80,
                          0x80, 0x10, clut & 0xFFFF, &buf, &buf, (short)s4, 1);
            return 0;
        }
        return 0;
    }
    return 0;
}
