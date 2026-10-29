/* room_m0429i — func_801982A4, blob offset 0x92BC, 0x670 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Near twin of room_m0269i func_8019070C (instruction-class similarity 0.92),
 * read from the retail disassembly. Differences: buf seed D_8018F234,
 * centre D_8019B678, colour curve D_8019B4FC; cmd 1 state 2 lasts 12 frames;
 * cmd 2 state 1 sets the D_800F3368 draw block (+0/+0xE/+0x10 = 0x40,
 * +2 = 4) and draws the glow around a stack vector {0, 0, f << 5, 0} with
 * size sin(f << 6) + 0x800; cmd 2 state 2 times the sprite by f*1024/12 and
 * draws it 1000 x 320 with CLUT x 0x10.
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

extern B4 D_8018F234;
extern GteShortVector D_8019B678;
extern unsigned char D_8019B4FC[];
extern short D_800942EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CFB7C(void *, short, void *);
extern void func_800783E4();
extern void func_800CF3AC();
extern void func_800D2B58();
extern void func_800CEE20();
extern void func_800D2370();

int func_801982A4(int mode, unsigned char *a1)
{
    GteShortVector v;
    GteShortVector d;
    GteShortVector g;
    B4 buf;
    int t;
    int size;
    int s2;
    int w;
    int clut;

    buf = D_8018F234;
    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0:
            H(a1, 0x12)++;
            H(a1, 8) += 0x10;
            H(a1, 0xA) += 8;
            if (SH(a1, 0x12) < 0x18)
                return 0;
            return 1;
        case 1:
            H(a1, 0x12)++;
            H(a1, 0) += H(a1, 8);
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 127 / 128;
            SH(a1, 0xC) = SH(a1, 0xC) * 127 / 128;
            {
                short y = SH(a1, 0xA) + 1;
                SH(a1, 0xA) = y;
                if (SH(a1, 2) >= D_800942EC)
                    SH(a1, 0xA) = -y;
            }
            if (SH(a1, 0x12) < 0x10)
                return 0;
            return 1;
        case 2:
            H(a1, 0x12)++;
            H(a1, 0xC) += 0x80;
            if (SH(a1, 0x12) < 0xC)
                return 0;
            return 1;
        }
        return 0;
    case 2:
        switch (SH(a1, 0x10)) {
        case 0:
            t = func_80077CF4((SH(a1, 0x12) << 10) / 24);
            t = (t + 0x1000) / 2;
            func_800CFB7C(a1 + 8, SH(a1, 6) * t / 4096, &v);
            v.x += D_8019B678.x;
            v.y += D_8019B678.y;
            v.z += D_8019B678.z;
            t = func_80077CF4((SH(a1, 0x12) << 10) / 24);
            func_800783E4(&D_8019B678, &v, 0x1000 - t, t, &d);
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            func_800CF3AC(D_8019B4FC, &buf, SH(a1, 0x12));
            func_800D2B58(&d, &v, 0, &buf, 0, 0x80, 1);
            return 0;
        case 1:
            size = func_80077CF4(SH(a1, 0x12) << 6) + 0x800;
            s2 = func_80077DC4(SH(a1, 0x12) * 1204 / 16) / 128 + 0x28;
            D_800F3368.a68 = 0x40;
            D_800F3368.a76 = 0x40;
            D_800F3368.a78 = 0x40;
            D_800F3368.a6A = 4;
            g.x = 0;
            g.y = 0;
            g.w = 0;
            g.z = SH(a1, 0x12) << 5;
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0x60, w);
            func_800CEE20(a1, &g, size, size, 0x40, clut & 0xFFFF, 1, s2 / 2, 0);
            return 0;
        case 2:
            s2 = func_80077DC4((SH(a1, 0x12) << 10) / 12) / 32;
            t = func_80077CF4((SH(a1, 0x12) << 10) / 12);
            func_800CFB7C(a1 + 8, SH(a1, 6) * t / 4096, &v);
            v.x += D_8019B678.x;
            v.y += D_8019B678.y;
            v.z += D_8019B678.z;
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            *(int *)&buf = 0x808080;
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 0;
            D_800F3368.a70 = D_800E2850[D_800E11EA];
            clut = func_80077AA4(0x10, D_800E120A);
            func_800D2370(&v, a1 + 8, 0x3E8, 0x140, 0x80, 0x90, 0x1F, 0x1F,
                          clut & 0xFFFF, &buf, &buf, (short)s2, 1);
            return 0;
        }
        return 0;
    }
    return 0;
}
