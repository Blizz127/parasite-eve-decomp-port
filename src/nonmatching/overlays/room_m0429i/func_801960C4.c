/* room_m0429i — func_801960C4, blob offset 0x70DC, 0x684 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback (same family as room_m0429i func_801982A4 / room_m0269i
 * func_8019070C).  Reads four seed blocks off the stack frame: b38 from the text
 * bytes at func_8018F210, b40 (a GteShortVector) from D_8018F1CC, b48 from
 * D_8018F21C, b50 from D_8018F220.
 * cmd 0: func_800CE8F0 position into b38, clears a1+8/+0xA, flips the byte at
 *        D_800F32D0->8->0->0x18 from 1 to 2 when D_800E2368->0xD is set, returns
 *        func_800CE560(D_800F33E0->8, 0x20, 0x20, func_8019549C).
 * cmd 1: counter a1+8 ++; on odd ticks below 20 spawns a func_800CE610 puff
 *        tagged with the spawn count a1+0xA; returns 2 once a1+8 >= 8.
 * cmd 2: copies a1+0..4 into a stack vector; ticks <= 0x18 draw func_800D0728
 *        sized func_80077CF4(f*1024/24) with the camera matrix D_800BCFA4[0];
 *        ticks < 0xD flash through func_800D1AE0; ticks <= 0x20 draw two
 *        func_800CEE20 sprites (0x3000 / 0x1800) and a 0x2000 one at half
 *        opacity, then a func_800D0728 ring (0x2BC x 0x3E8); finally the
 *        D_800F3368 draw block is reset (the last 0x20 write set is common).
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
typedef struct { unsigned char c[8]; } B8;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern unsigned char func_8018F210[];
extern B8 D_8018F1CC;
extern B4 D_8018F21C;
extern B4 D_8018F220;
extern unsigned char func_8019549C[];
extern unsigned short D_800E11EA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern void *D_800F33E0;
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80077CF4();
extern int func_80077AA4();
extern int func_800CE560();
extern void *func_800CE610();
extern void func_800CE8F0();
extern void func_800CEE20();
extern void func_800D0728();
extern void func_800D1AE0();

int func_801960C4(int mode, unsigned char *a1)
{
    GteShortVector v;   /* frame order = retail sp+0x30, 0x38, 0x40, 0x48, 0x50 */
    B8 b38;
    GteShortVector b40;
    struct { B4 b; B4 hole; } b48;   /* retail leaves sp+0x4C unwritten */
    B4 b50;
    int a, t, s3, s0, w, clut;
    unsigned char *p;
    unsigned char *q;
    unsigned char *r;
    unsigned short n;

    b38 = *(B8 *)func_8018F210;
    b40 = *(GteShortVector *)&D_8018F1CC;
    b48.b = D_8018F21C;
    b50 = D_8018F220;
    switch (mode) {
    case 0:
        func_800CE8F0(P(D_800F32D0, 8), 0, &b38, a1);
        SH(a1, 8) = 0;
        SH(a1, 0xA) = 0;
        if (D_800E2368[0xD] != 0) {
            p = P(D_800F32D0, 8);
            if (p != 0) {
                q = *(unsigned char **)p;
                if (q != 0) {
                    r = P(q, 0x18);
                    if (*r == 1)
                        *r = 2;
                }
            }
        }
        return func_800CE560(P(D_800F33E0, 8), 0x20, 0x20, func_8019549C);
    case 1:
        n = H(a1, 8) + 1;
        H(a1, 8) = n;
        if (SH(a1, 0xA) < 0x14 && (n & 1)) {
            q = func_800CE610(P(D_800F33E0, 8));
            if (q != 0) {
                SH(q, 0x1C) = 0;
                SH(q, 0x1E) = 0;
                SH(q, 6) = H(a1, 0xA);
            }
            H(a1, 0xA) = H(a1, 0xA) + 1;
        }
        if (SH(a1, 8) < 8)
            return 0;
        return 2;
    case 2:
        v.x = H(a1, 0);
        v.y = H(a1, 2);
        v.z = H(a1, 4);
        a = SH(a1, 8);
        if (a < 0x19) {
            s3 = 0x80 - (a << 7) / 24;
            t = func_80077CF4((a << 10) / 24);
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            func_800D0728(&v, 0x6A4, 0x9C4, 0x16, 0, t, t, 0, &b48, s3, 1);
            a = SH(a1, 8);
        }
        if (a < 0xD)
            func_800D1AE0(&b48, 0x80 - (a << 7) / 12, 1, 8);
        if (SH(a1, 8) < 0x21) {
            D_800F3368.a68 = 0x20;
            D_800F3368.a6A = 2;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            D_800F3368.a6C = 3;
            D_800F3368.a74 = 0x32;
            D_800F3368.a6E = 0;
            D_800F3368.a72 = 0;
            D_800F3368.a70 = D_800E2850[D_800E11EA];
            a = SH(a1, 8);
            s3 = 0x80 - a * 4;
            b40.z = a << 4;
            clut = func_80077AA4(0x10, D_800E120A);
            func_800CEE20(&v, &b40, 0x3000, 0x3000, 0x98, clut & 0xFFFF, 1, s3, 0);
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0x70, w);
            func_800CEE20(&v, &b40, 0x1800, 0x1800, 0x98, clut & 0xFFFF, 1, s3, 0);
            D_800F3368.a68 = 0x40;
            D_800F3368.a6A = 4;
            D_800F3368.a76 = 0x40;
            D_800F3368.a78 = 0x40;
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0x90, w);
            s0 = s3 / 2;
            func_800CEE20(&v, 0, 0x2000, 0x2000, 8, clut & 0xFFFF, 1, s0, 0);
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            t = (SH(a1, 8) << 5) + 0x1000;
            func_800D0728(&v, 0x2BC, 0x3E8, 0x14, 0, t, t, 0, &b50, s0, 1);
        }
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 8;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        return 0;
    }
    return 0;
}
