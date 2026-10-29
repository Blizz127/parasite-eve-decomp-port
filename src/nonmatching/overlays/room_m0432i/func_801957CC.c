/* room_m0432i — func_801957CC, blob offset 0x67E4, 0x374 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback (a0 = command 1/2, a1 = effect record), a sibling of
 * room_m0429i func_801982A4. Command 1 steps the record (+8/+0xA velocity
 * and the +0x12 frame counter): state 0 runs 12 frames (+8, +0xC), state 1
 * runs 24 frames (+0x1D, +0xB). Command 2 state 0 draws via func_800D0E88
 * with size sin(f*1024/12)/8; state 1 places an 8-byte vector copied from
 * D_8018F1F4 (z = f << 5) at an orbit point and draws it through func_800CEE20.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot macro. */
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

extern GteShortVector D_8018F1F4;
extern unsigned char D_80199434[];
extern MATRIX *D_800BCFA4[];
extern unsigned short D_800E11FA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CFB7C(void *, short, void *);
extern void func_800CF3AC();
extern void func_800D0E88();
extern void func_800CEE20();

int func_801957CC(int mode, unsigned char *a1)
{
    GteShortVector g;
    GteShortVector v;
    unsigned char m[8];
    int t;
    int s2;
    int a;
    int w;
    int clut;

    g = D_8018F1F4;
    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0:
            H(a1, 0x12)++;
            H(a1, 8) += 8;
            H(a1, 0xA) += 0xC;
            if (SH(a1, 0x12) < 0xC)
                return 0;
            return 1;
        case 1:
            H(a1, 0x12)++;
            H(a1, 8) += 0x1D;
            H(a1, 0xA) += 0xB;
            if (SH(a1, 0x12) < 0x18)
                return 0;
            return 1;
        }
        return 0;
    case 2:
        switch (SH(a1, 0x10)) {
        case 0:
            t = func_80077DC4((SH(a1, 0x12) << 10) / 12);
            s2 = t / 8;
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            func_800CF3AC(D_80199434, m, SH(a1, 0x12));
            func_800D0E88(a1, a1 + 8, 0x7D0, s2, m, 0, 0, 0x80, 1);
            return 0;
        case 1:
            a = (SH(a1, 0x12) << 10) / 24;
            t = func_80077DC4(a) * SH(a1, 6) / 4096;
            func_800CFB7C(a1 + 8, t, &v);
            v.x += H(a1, 0);
            v.y += H(a1, 2);
            v.z += H(a1, 4);
            s2 = func_80077DC4(a) * 2;
            g.z = SH(a1, 0x12) << 5;
            w = func_80077CF4(a) / 32;
            D_800F3368.a68 = 0x20;
            D_800F3368.a6A = 2;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 1;
            D_800F3368.a70 = D_800E2850[D_800E11FA];
            clut = func_80077AA4(0, D_800E120A + 4);
            func_800CEE20(&v, &g, s2, s2, D_800F3368.a6A + 0x64, clut & 0xFFFF, 1, w, 0);
            return 0;
        }
        return 0;
    }
    return 0;
}
