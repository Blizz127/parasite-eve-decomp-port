/* room_m0429i — func_801972EC, blob offset 0x8304, 0x600 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback, modes 1/2 on a record a1, read from the retail disassembly.
 * Mode 1 steps the record by state (+0x10): 0 adds +8/+0xA/+0xC to +0/+2/+4
 * and damps +8/+0xC by 31/32 (bounces +0xA when +2 >= D_800942EC), done at
 * frame (+0x12) 0x10; 1 does the same with 127/128 and +0xA - 1, done at
 * 0x18; 2 only counts to 0x18. Mode 2 draws: state 0 a func_800CEE20 glow
 * sized func_80077CF4(f << 7) * 3 / 2 (draw block D_800F3368 set to 0x20,
 * +2 = 2); state 1 the same with f*1024/24 through func_80077DC4 and the
 * stack vector buf3; state 2 loads the GTE rotation/translation from
 * D_800BCFA4[0] and draws func_800D0728 sized func_80077CF4(f*1024/24),
 * timed by func_80077DC4(f*1024/24) / 32.
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

extern GteShortVector D_8018F1CC;
extern GteShortVector D_8018F1D4;
extern B4 D_8018F234;
extern unsigned char D_8019B484[];
extern short D_800942EC;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CF3AC();
extern void func_800CEE20();
extern void func_800D0728();

int func_801972EC(int mode, unsigned char *a1)
{
    GteShortVector g;
    GteShortVector buf2;
    B4 buf3;
    int s1;
    int s2;
    int t;
    int w;
    int clut;

    g = D_8018F1CC;
    buf2 = D_8018F1D4;
    buf3 = D_8018F234;
    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0:
            H(a1, 0) += H(a1, 8);
            H(a1, 0x12)++;
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 31 / 32;
            SH(a1, 0xC) = SH(a1, 0xC) * 31 / 32;
            t = H(a1, 0xA);
            H(a1, 0xA) = t;
            if (SH(a1, 2) >= D_800942EC)
                SH(a1, 0xA) = -(short)t;
            if (SH(a1, 0x12) < 0x10)
                return 0;
            return 1;
        case 1:
            H(a1, 0) += H(a1, 8);
            H(a1, 0x12)++;
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 127 / 128;
            SH(a1, 0xC) = SH(a1, 0xC) * 127 / 128;
            t = H(a1, 0xA) - 1;
            H(a1, 0xA) = t;
            if (SH(a1, 2) >= D_800942EC)
                SH(a1, 0xA) = -(short)t;
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
            s1 = func_80077CF4(SH(a1, 0x12) << 7) * 3 / 2;
            t = func_80077CF4(SH(a1, 0x12) << 7);
            s2 = t / 160;
            D_800F3368.a68 = 0x20;
            D_800F3368.a6A = 2;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            g.z = -(SH(a1, 0x12) << 5);
            func_800CF3AC(D_8019B484, &buf3, SH(a1, 0x12));
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0x90, w);
            func_800CEE20(a1, &g, s1, s1, 0x98, clut & 0xFFFF, 2, s2, 0);
            return 0;
        case 1:
            s1 = func_80077DC4((SH(a1, 0x12) << 10) / 24);
            D_800F3368.a68 = 0x20;
            D_800F3368.a6A = 2;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            g.z = SH(a1, 0x12) << 5;
            func_800CF3AC(D_8019B484, &buf3, SH(a1, 0x12));
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0x80, w);
            func_800CEE20(a1, &g, s1, s1, 0x98, clut & 0xFFFF, 1, 0x80, &buf3);
            return 0;
        case 2:
            s2 = func_80077DC4((SH(a1, 0x12) << 10) / 24) / 32;
            s1 = func_80077CF4((SH(a1, 0x12) << 10) / 24);
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            func_800CF3AC(D_8019B484, &buf3, SH(a1, 0x12) << 1);
            func_800D0728(a1, 0x320, 0x578, 0x18, &buf2, s1, s1, 0, &buf3, s2, 1);
            return 0;
        }
        return 0;
    }
    return 0;
}
