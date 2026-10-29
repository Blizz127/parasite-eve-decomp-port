/* room_m0432i — func_80198348, blob offset 0x9360, 0xBFC bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback, modes 0/1/2 on a record a1 (kind +0x10, frame +0x12,
 * position +0/+2/+4), a2 = spawn parameter block; read from the retail
 * disassembly (same family as func_8019702C / func_801978EC).
 *   mode 0: clears kind/frame, plays the D_800B0E64 voice (0x5DA) when it
 *           exists, looks up the two sprite handles D_80199508 / D_8019950C
 *           through func_8006E498 (keys 0xC58C4704 / 0xC5CC4704, each reset
 *           with func_800C6D5C), copies the a2 position and registers the
 *           func_801981B0 child through func_800CE560 (its result is returned).
 *   mode 1: frame +0x12 counts up. On frame 6, if func_800C6B90(a1, a2->8)
 *           hits and the D_800E2368 record has byte +0xD set and the
 *           D_800F32D0 object's flag word reads 0x01xxxxxx, the D_8009D254
 *           state gets bit 0x4000 and the object's flag word is rewritten
 *           (0x21 in bits 24..29) and gets bit 31. While the frame is < 7,
 *           four particles are taken from the func_800CE610 pool at
 *           D_800F33E0[2] and scattered with four func_80071A54 draws.
 *           Returns 1 from frame 0x28 on.
 *   mode 2: loads the camera GTE matrix from D_800BCFA4[0]; frames < 4 flash
 *           the screen white (func_800D1AE0 colour 0xFFFFFF, 0x6E), frames
 *           4..5 and 6..12 fade with buf D_8018F228 / sine; sets the
 *           D_800F3368 draw block and draws a 0x2000 sprite (func_800CEE20),
 *           two glows (func_800D004C / func_800D0728), then two scaled-matrix
 *           sprites through func_800C6EF8..6F4C on D_80199508, and, for frame
 *           < 25, a third on D_8019950C. Always ends by resetting the draw
 *           block; returns 0.
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
#define W(o, x) (*(int *)((char *)(o) + (x)))

extern GteShortVector D_8018F1FC;
extern B4 D_8018F228;
extern unsigned char D_8019948C[];
extern int D_80199508;
extern int D_8019950C;
extern int D_800B0E64;
extern unsigned short D_800942EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E11FA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern int **D_8009D254;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern unsigned char func_801981B0[];
extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern int func_80077A64();
extern int func_8006E498();
extern int func_8006DF50();
extern int func_800D3FD8();
extern int func_800CE560();
extern int func_800C6B90();
extern unsigned char *func_800CE610();
extern void func_800C6D5C();
extern void func_800D1AE0();
extern void func_800CEE20();
extern void func_800CF3AC();
extern void func_800D004C();
extern void func_800D0728();
extern void func_800C6EC0();
extern void func_800C6ED8();
extern void func_80079754();
extern void func_80078CC4();
extern void func_800C6EF8();
extern void func_800C6FA0();
extern void func_800C71E4();
extern void func_800C6F4C();

int func_80198348(int mode, unsigned char *a1, unsigned char *a2)
{
    GteShortVector p30;         /* retail sp+0x30 */
    GteShortVector seed8;       /* sp+0x38 */
    GteShortVector rot;         /* sp+0x40 */
    struct { B4 col; B4 gap; B4 buf4; } L;   /* retail: col at sp+0x48, buf4 at sp+0x50 */
    GteShortVector v58;         /* sp+0x58 */
    GteShortVector v60;         /* sp+0x60 */
    MATRIX m1;                  /* sp+0x68 */
    int sc1[3];                 /* sp+0x88 */
    MATRIX m2;                  /* sp+0x98 */
    int sc2[3];                 /* sp+0xB8 */
    MATRIX m3;                  /* sp+0xC8 */
    int sc3[3];                 /* sp+0xE8 */
    unsigned char *p;
    int i;
    int r;
    int f;
    int q;
    int s3;
    int s5;
    int w;
    int clut;
    int c;

    seed8 = D_8018F1FC;
    L.buf4 = D_8018F228;
    switch (mode) {
    case 0:
        SH(a1, 0x10) = 0;
        SH(a1, 0x12) = 0;
        if (D_800B0E64 != 0) {
            r = func_800D3FD8();
            func_8006DF50(D_800B0E64, 0x5DA, r, 0x80, 0x7F);
        }
        r = func_8006E498(D_800B0E64, 0xC58C4704);
        D_80199508 = r;
        func_800C6D5C(r, 0, 0);
        r = func_8006E498(D_800B0E64, 0xC5CC4704);
        D_8019950C = r;
        func_800C6D5C(r, 0, 0);
        H(a1, 0) = H(a2, 0);
        H(a1, 2) = H(a2, 2);
        H(a1, 4) = H(a2, 4);
        return func_800CE560(W(D_800F33E0, 8), 0xC, 0x20, func_801981B0);
    case 1:
        H(a1, 0x12)++;
        if (SH(a1, 0x12) == 6) {
            if (func_800C6B90(a1, W(a2, 8)) != 0 && D_800E2368[0xD] != 0) {
                unsigned char *o = D_800F32D0;
                int *wp;
                if ((***(int ***)(o + 8) & 0x3F000000) == 0x01000000) {
                    wp = *D_8009D254;
                    wp[0x4C / 4] |= 0x4000;
                    wp = *(int **)W(o, 8);
                    *wp = (*wp & 0xC0FFFFFF) | 0x21000000;
                    wp = *(int **)W(o, 8);
                    *wp |= 0x80000000;
                }
            }
        }
        if (SH(a1, 0x12) < 7) {
            for (i = 0; i < 4; i++) {
                p = func_800CE610(W(D_800F33E0, 8));
                if (p != 0) {
                    H(p, 0) = H(a1, 0);
                    H(p, 2) = H(a1, 2);
                    H(p, 4) = H(a1, 4);
                    r = func_80071A54();
                    H(p, 6) = (r & 0xFFF) + 0x800;
                    r = func_80071A54();
                    H(p, 2) = H(p, 2) - (r & 0x7F);
                    r = func_80071A54();
                    H(p, 0) = H(p, 0) + (r % 0x1000 - 0x800);
                    r = func_80071A54();
                    H(p, 8) = 0;
                    H(p, 0xA) = 0;
                    H(p, 4) = H(p, 4) + (r % 0x1000 - 0x800);
                }
            }
        }
        if (SH(a1, 0x12) < 0x28)
            return 0;
        return 1;
    case 2:
        p30.x = H(a1, 0);
        p30.y = H(a1, 2);
        p30.z = H(a1, 4);
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x29;
        f = SH(a1, 0x12);
        if (f < 4) {
            *(int *)&L.col = 0xFFFFFF;
            func_800D1AE0(&L.col, 0x6E, 2, 8);
        } else if (f < 6) {
            func_800D1AE0(&L.buf4, 0x80, 1, 8);
        } else if (f < 13) {
            r = func_80077DC4(((f - 6) << 10) / 6);
            func_800D1AE0(&L.buf4, r / 64, 1, 8);
        }
        r = func_80077DC4((SH(a1, 0x12) << 10) / 40);
        s5 = r / 32;
        if (H(a1, 0x12) & 1)
            s5 = s5 * 3 / 4;
        D_800F3368.a68 = 0x20;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a6A = 2;
        v58.x = p30.x;
        v58.z = p30.z;
        v58.y = D_800942EC;
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        v60.x = 0x400;
        v60.y = 0;
        v60.z = 0;
        v60.w = 1;
        D_800F3368.a70 = D_800E2850[D_800E11FA];
        clut = func_80077AA4(0, D_800E120A + 4);
        func_800CEE20(&v58, &v60, 0x2000, 0x2000, 0x64, clut & 0xFFFF, 1, s5, 0);
        func_800CF3AC(D_8019948C, &L.col, SH(a1, 0x12));
        func_800D004C(&p30, 0x4B0, 0x5DC, 0x10, 0, 0x1000, 0x1000, &L.col, 0, s5, 1);
        q = (SH(a1, 0x12) << 10) / 40;
        s3 = func_80077CF4(q);
        func_800D0728(&p30, 0xBB8, 0x1770, 0x12, &seed8, s3, s3, 0, &L.col, s5, 1);
        D_800F3368.a74 = 4;
        r = func_80077DC4(q);
        s5 = r / 32;
        s3 = func_80077CF4(q) + 0x800;
        rot.x = 0;
        r = func_80077CF4(q);
        rot.y = r;
        rot.z = 0;
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        r = func_80077A64(0, 1, 0, 0);
        c = D_800E2850[D_800E11EA] | r;
        w = D_800E1204[D_800F3368.a6C];
        if (D_800F3368.a6C == 4 && D_800F3428 != 0)
            w += 4;
        clut = func_80077AA4(0x10, w);
        func_800C6EC0(c & 0xFFFF, clut & 0xFFFF);
        func_800C6ED8(1);
        func_80079754(&rot, &m1);
        sc1[0] = s3;
        sc1[1] = s3 * 2;
        sc1[2] = s3;
        m1.t[0] = p30.x;
        m1.t[1] = p30.y;
        m1.t[2] = p30.z;
        func_80078CC4(&m1, sc1);
        func_800C6EF8(D_80199508);
        func_800C6FA0(D_80199508, (s5 * 3 / 2) & 0xFFFF);
        func_800C71E4(D_80199508, &m1);
        func_800C6F4C(D_80199508);
        s3 = s3 / 2;
        rot.x = 0;
        r = func_80077CF4(q);
        rot.y = r * 2;
        rot.z = 0;
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        r = func_80077A64(0, 1, 0, 0);
        c = D_800E2850[D_800E11EA] | r;
        w = D_800E1204[D_800F3368.a6C];
        if (D_800F3368.a6C == 4 && D_800F3428 != 0)
            w += 4;
        clut = func_80077AA4(0x10, w);
        func_800C6EC0(c & 0xFFFF, clut & 0xFFFF);
        func_800C6ED8(1);
        func_80079754(&rot, &m2);
        sc2[0] = s3;
        sc2[1] = s3 * 8;
        sc2[2] = s3;
        m2.t[0] = p30.x;
        m2.t[1] = p30.y;
        m2.t[2] = p30.z;
        func_80078CC4(&m2, sc2);
        func_800C6EF8(D_80199508);
        func_800C6FA0(D_80199508, s5 & 0xFFFF);
        func_800C71E4(D_80199508, &m2);
        func_800C6F4C(D_80199508);
        f = SH(a1, 0x12);
        if (f < 25) {
            q = (f << 10) / 24;
            r = func_80077DC4(q);
            s5 = r / 32;
            s3 = func_80077DC4(q);
            rot.x = 0;
            rot.z = 0;
            rot.y = SH(a1, 0x12) * 40;
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 0;
            D_800F3368.a70 = D_800E2850[D_800E11EA];
            r = func_80077A64(0, 1, 0, 0);
            c = D_800E2850[D_800E11EA] | r;
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0x30, w);
            func_800C6EC0(c & 0xFFFF, clut & 0xFFFF);
            func_800C6ED8(1);
            func_80079754(&rot, &m3);
            sc3[0] = s3;
            sc3[1] = 0x1000;
            sc3[2] = s3;
            m3.t[0] = p30.x;
            m3.t[1] = p30.y;
            m3.t[2] = p30.z;
            func_80078CC4(&m3, sc3);
            func_800C6EF8(D_8019950C);
            func_800C6FA0(D_8019950C, s5 & 0xFFFF);
            func_800C71E4(D_8019950C, &m3);
            func_800C6F4C(D_8019950C);
        }
        D_800F3368.a68 = 0x10;
        D_800F3368.a6A = 1;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        return 0;
    }
    return 0;
}
