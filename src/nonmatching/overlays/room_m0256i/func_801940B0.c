/* room_m0256i — func_801940B0, blob offset 0x50C8, 0x1270 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Particle effect callback, read from the retail disassembly. Mode (a0) 1 steps
 * the object at a1 (state at a1+0x14, frame counter a1+0x16): state 0 seeds the
 * position from the pool object (func_800CE8F0/9D4) and spawns a child, state 1
 * is the main flight (grow/scale curve, respawn children every 3rd frame, a
 * sound/flag trigger when func_800C6B90 fires), states 2..5 are timed bounces
 * and fades. Mode 2 draws it (state a1+0x14 - 1): 0 = glow pair around the
 * object, 1 = two-polygon spin, 2 = scaled sprite (func_80078CC4), 3..4 = glow
 * quads with the D_800F3368 draw block set up per state.
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
typedef struct { short r[10]; int t[3]; } RMtx;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern B4 D_8018F218;
extern B4 D_8018F21C;
extern GteShortVector D_8018F220;
extern GteShortVector D_8018F1CC;
extern unsigned char D_80195EFC[];
extern int D_80195EF0;
extern int D_80195EF8;
extern short D_800942EC;
extern unsigned int ***D_800F32D0;
extern unsigned int ***D_800F33E0;
extern unsigned char **D_8009D254;
extern unsigned char *D_800E2368;
extern int D_800E27EC;
extern unsigned short D_800E11E8;
extern unsigned short D_800E11EA;
extern unsigned short D_800E1208;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern int func_80077A64();
extern int func_800C6B90();
extern int func_800CE8F0();
extern int func_800CE9D4();
extern int func_800CE870();
extern unsigned char *func_800CE610();
extern int func_800CFAA8();
extern void func_800CFD50(void *, void *, unsigned short);
extern void func_800CFB7C(void *, short, void *);
extern int func_800D3AFC();
extern int func_800D3114();
extern int func_800D004C();
extern int func_800CEE20();
extern int func_80079754();
extern int func_80078CC4();
extern int func_800C6EC0();
extern int func_800C6ED8();
extern int func_800C6EF8();
extern int func_800C7098();
extern int func_800C6FA0();
extern int func_800C71E4();
extern int func_800C6F4C();

/* the shared "kick the screen-flash handshake" block (state 1 and state 5) */
#define KICK() do { \
    if (D_800E2368[0xD]) { \
        unsigned int ***p_ = D_800F32D0; \
        if ((**p_[2] & 0x3F000000) == 0x01000000) { \
            unsigned int *q_; \
            *(unsigned int *)(*D_8009D254 + 0x4C) |= 0x4000; \
            q_ = *p_[2]; \
            *q_ = (*q_ & 0xC0FFFFFF) | 0x19000000; \
            q_ = *p_[2]; \
            *q_ |= 0x80000000; \
        } \
    } \
} while (0)

#define CLUT_OFS(w, st) do { \
    (w) = D_800E1204[D_800F3368.a6C]; \
    if (D_800F3368.a6C == 4 && D_800F3428 != 0) \
        (w) += 4; \
} while (0)

int func_801940B0(int mode, unsigned char *a1)
{
    B4 b38;
    B4 b40;
    GteShortVector b48;
    GteShortVector b50;
    short v58[4];
    short v60[4];
    RMtx mt;
    int sc[3];
    unsigned char *s1;
    int s2, s5, t, w, clut, r;

    b38 = D_8018F218;
    b40 = D_8018F21C;
    b48 = D_8018F220;
    b50 = D_8018F1CC;
    if (mode == 1) {
        switch (SH(a1, 0x14)) {
        case 0:
            SH(a1, 0x16) = 0;
            SH(a1, 0x14) = 1;
            SH(a1, 0xE) = 0;
            func_800CE8F0(D_800F32D0[2], 2, &b48, a1);
            func_800CE9D4(D_800F32D0[2], 0, v60);
            v60[0] = v60[0] + 0x200;
            v60[1] = v60[1] - 0x200 + (SH(a1, 6) << 9);
            H(a1, 8) = v60[0];
            H(a1, 0xA) = v60[1];
            H(a1, 0xC) = H(v60, 4);
            H(a1, 0xA) = H(a1, 0xA) + 0x800;
            func_800D3AFC(D_80195EFC + SH(a1, 6) * 136, 0x10, a1, 1);
            H(a1, 0x10) = (func_80071A54() & 7) + 8;
            SH(a1, 0x12) = 0;
            s1 = func_800CE610(D_800F33E0[2]);
            if (s1 == 0)
                return 0;
            H(s1, 0) = H(a1, 0);
            H(s1, 2) = H(a1, 2);
            SH(s1, 0x14) = 4;
            SH(s1, 0x16) = 0;
            H(s1, 4) = H(a1, 4);
            return 0;
        case 1:
            func_800CE870(D_8009D254, 1, v58);
            H(a1, 0x16) = H(a1, 0x16) + 1;
            s1 = D_80195EFC + SH(a1, 6) * 136;
            func_800CFAA8(a1, v58, v60);
            if (SH(a1, 0xE) == 0) {
                if (SH(a1, 0x10) >= 0x1B) {
                    if (SH(a1, 0x12) < 0x800)
                        SH(a1, 0x12) = SH(a1, 0x12) + 0x50;
                    if (SH(a1, 0x12) > 0x800)
                        SH(a1, 0x12) = 0x800;
                }
                t = func_80077CF4(SH(a1, 0x12));
                t = t * D_80195EF0 / 4096;
                func_800CFD50(v60, a1 + 8, t & 0xFFFF);
                if (SH(a1, 0x10) < 0x32)
                    SH(a1, 0x10) = SH(a1, 0x10) + 6;
                t = SH(a1, 0x10) * SH(a1, 0x10);
                func_800CFB7C(a1 + 8, (short)(t / 16), v58);
                H(a1, 0) = H(a1, 0) + H(v58, 0);
                H(a1, 2) = H(a1, 2) + H(v58, 2);
                H(a1, 4) = H(a1, 4) + H(v58, 4);
            } else {
                short e = SH(a1, 0xE) + 1;
                SH(a1, 0xE) = e;
                if (e >= 0x10)
                    return 1;
            }
            func_800D3AFC(s1, 0x10, a1, 0);
            if (func_800C6B90(a1, 0x64)) {
                if (SH(a1, 2) >= D_800942EC - 0x202)
                    KICK();
            }
            if (SH(a1, 0xE) == 0) {
                if (D_800E27EC % 3 == 0) {
                    s1 = func_800CE610(D_800F33E0[2]);
                    if (s1) {
                        H(s1, 0) = H(a1, 0);
                        H(s1, 2) = H(a1, 2);
                        H(s1, 4) = H(a1, 4);
                        H(s1, 8) = H(a1, 8);
                        H(s1, 0xA) = H(a1, 0xA);
                        SH(s1, 0x16) = 0;
                        H(s1, 8) = H(s1, 8) + 0x800;
                        SH(s1, 0x14) = 3;
                        H(s1, 0xC) = H(a1, 0xC);
                    }
                }
                s1 = func_800CE610(D_800F33E0[2]);
                if (s1) {
                    r = func_80071A54() & 3;
                    H(s1, 0) = H(D_80195EFC, (SH(a1, 6) * 17 + r) * 8 + 0x20);
                    r = func_80071A54() & 3;
                    H(s1, 2) = H(D_80195EFC, (SH(a1, 6) * 17 + r) * 8 + 0x22);
                    r = func_80071A54() & 3;
                    H(s1, 4) = H(D_80195EFC, (SH(a1, 6) * 17 + r) * 8 + 0x24);
                    H(s1, 0) = H(s1, 0) - 0x40 + (func_80071A54() & 0x7F);
                    H(s1, 2) = H(s1, 2) - 0x40 + (func_80071A54() & 0x7F);
                    H(s1, 4) = H(s1, 4) - 0x40 + (func_80071A54() & 0x7F);
                    SH(s1, 8) = func_80071A54() % 60 - 0x1E;
                    SH(s1, 0xA) = func_80071A54() % 60 - 0x1E;
                    SH(s1, 0x14) = 2;
                    SH(s1, 0x16) = 0;
                    SH(s1, 0xC) = func_80071A54() % 60 - 0x1E;
                }
            }
            if (SH(a1, 2) < D_800942EC) {
                if (D_800E27EC < 0x51)
                    return 0;
            }
            if (SH(a1, 0xE) != 0)
                return 0;
            s1 = func_800CE610(D_800F33E0[2]);
            if (s1) {
                H(s1, 0) = H(a1, 0);
                H(s1, 2) = H(a1, 2);
                H(s1, 4) = H(a1, 4);
                SH(s1, 0x14) = 5;
                SH(s1, 0x16) = 0;
                H(s1, 2) = D_800942EC;
            }
            SH(a1, 0xE) = 1;
            return 0;
        case 2:
            H(a1, 0) = H(a1, 0) + H(a1, 8);
            H(a1, 0x16) = H(a1, 0x16) + 1;
            H(a1, 2) = H(a1, 2) + H(a1, 0xA);
            H(a1, 4) = H(a1, 4) + H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 63 / 64;
            SH(a1, 0xC) = SH(a1, 0xC) * 63 / 64;
            if (!(SH(a1, 2) < D_800942EC))
                SH(a1, 0xA) = -(short)H(a1, 0xA);
            return SH(a1, 0x16) < 0x10 ? 0 : 1;
        case 3:
            H(a1, 0x16) = H(a1, 0x16) + 1;
            return SH(a1, 0x16) < 0xA ? 0 : 1;
        case 4:
            H(a1, 0x16) = H(a1, 0x16) + 1;
            return SH(a1, 0x16) < 0x10 ? 0 : 1;
        case 5:
            H(a1, 0x16) = H(a1, 0x16) + 1;
            if (func_800C6B90(a1, 0xB4)) {
                if (SH(a1, 0x16) < 0x10)
                    KICK();
            }
            return SH(a1, 0x16) < 0x18 ? 0 : 1;
        }
        return 0;
    }
    if (mode == 2) {
        switch ((short)(H(a1, 0x14) - 1)) {
        case 0:
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            CLUT_OFS(w, 0);
            clut = func_80077AA4(0x30, w);
            func_800D3114(D_80195EFC + SH(a1, 6) * 136, 0x10, 0xF0, 0, 0x60, 0xFF, 0xF,
                          D_800E2850[D_800E11EA], clut & 0xFFFF, 0x80, &b38, &b40, 1);
            s5 = ((D_800E27EC & 1) << 5) + 0x60;
            func_800D004C(a1, 0x190, 0x190, 8, 0, 0x1000, 0x1000, &b38, 0, s5, 1);
            r = func_80071A54() & 7;
            func_800D004C(D_80195EFC + SH(a1, 6) * 136 + r * 8, 0x1F4, 0x1F4, 6, 0, 0x1000,
                          0x1000, &b38, 0, (unsigned int)s5 >> 1, 1);
            return 0;
        case 1:
            s5 = func_80077DC4(SH(a1, 0x16) << 6) / 32;
            s2 = func_80077DC4(SH(a1, 0x16) << 6);
            D_800F3368.a68 = 0x10;
            D_800F3368.a6A = 1;
            D_800F3368.a76 = 0x10;
            D_800F3368.a78 = 0x10;
            D_800F3368.a6C = 2;
            D_800F3368.a6E = 0;
            D_800F3368.a70 = D_800E2850[D_800E11E8];
            clut = func_80077AA4(0x20, D_800E1208);
            func_800CEE20(a1, 0, s2, s2, D_800F3368.a6A * 2 + 0xD8, clut & 0xFFFF, 1, s5, &b38);
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 0;
            D_800F3368.a70 = D_800E2850[D_800E11EA];
            return 0;
        case 2:
            t = func_80077CF4((SH(a1, 0x16) << 10) / 10);
            s2 = t / 4;
            t = func_80077CF4((SH(a1, 0x16) << 11) / 10);
            s5 = t / 48;
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 0;
            D_800F3368.a70 = D_800E2850[D_800E11EA];
            t = func_80077A64(0, 1, 0, 0);
            w = (D_800E2850[D_800E11EA] | t) & 0xFFFF;
            r = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                r += 4;
            clut = func_80077AA4(0x70, r);
            func_800C6EC0(w, clut & 0xFFFF);
            func_800C6ED8(1);
            func_80079754(a1 + 8, &mt);
            mt.t[0] = SH(a1, 0);
            mt.t[1] = SH(a1, 2);
            mt.t[2] = SH(a1, 4);
            sc[0] = s2;
            sc[1] = s2;
            sc[2] = 0x2AA;
            func_80078CC4(&mt, sc);
            func_800C6EF8(D_80195EF8);
            func_800C7098(D_80195EF8, b40.c[0], b40.c[1], b40.c[2]);
            func_800C6FA0(D_80195EF8, s5 & 0xFFFF);
            func_800C71E4(D_80195EF8, &mt);
            func_800C6F4C(D_80195EF8);
            return 0;
        case 3:
            t = func_80077CF4(SH(a1, 0x16) << 6);
            s2 = t / 2 + 0x1000;
            s5 = func_80077DC4(SH(a1, 0x16) << 6) / 32;
            D_800F3368.a68 = 0x40;
            D_800F3368.a76 = 0x40;
            D_800F3368.a78 = 0x40;
            D_800F3368.a6A = 4;
            D_800F3368.a74 = 0x18;
            CLUT_OFS(w, 0);
            clut = func_80077AA4(0x60, w);
            func_800CEE20(a1, 0, s2, s2, 0x7C, clut & 0xFFFF, 1, s5, 0);
            D_800F3368.a74 = 8;
            return 0;
        case 4:
            s2 = func_80077CF4(SH(a1, 0x16) << 6) + 0x1000;
            func_80077CF4((SH(a1, 0x16) << 11) / 24);
            D_800F3368.a68 = 0x20;
            D_800F3368.a6A = 2;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            CLUT_OFS(w, 0);
            clut = func_80077AA4(0x10, w);
            t = (short)(SH(a1, 0x16) / 3);
            func_800CEE20(a1, &b50, s2 * 3 / 2, s2 * 3 / 2, D_800F3368.a6A * t + 0x20,
                          clut & 0xFFFF, 1, 0x80, 0);
            if (SH(a1, 0x16) >= 0xD)
                return 0;
            s2 = func_80077CF4((SH(a1, 0x16) << 10) / 12) * 4;
            s5 = func_80077DC4((SH(a1, 0x16) << 10) / 12) / 32;
            D_800F3368.a68 = 0x20;
            D_800F3368.a6A = 2;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            D_800F3368.a72 = 5;
            CLUT_OFS(w, 0);
            clut = func_80077AA4(0x80, w);
            func_800CEE20(a1, &b50, s2, s2, 0x92, clut & 0xFFFF, 1, s5, &b40);
            D_800F3368.a72 = 0;
            return 0;
        }
        return 0;
    }
    return 0;
}
