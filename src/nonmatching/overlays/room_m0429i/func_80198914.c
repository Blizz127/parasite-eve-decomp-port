/* room_m0429i — func_80198914, blob offset 0x992C, 0x8E4 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback, modes 0/1/2 on a record a1, read from the retail
 * disassembly. Mode 0 clears a1+8/+0xA, plays sound 0x5DF/0x5E0 and arms
 * func_801982A4 (0x14, 0x3C); returns func_800CE560's result. Mode 1 (once,
 * while the global frame counter D_800E27EC < 4) spawns 4 + 7 + 5 puffs through
 * func_800CE610 (copy a1+0..4; kind 0: random +-16 velocity, kind 1: random
 * velocity and size (r & 0x3FF) + 0x5DC, kind 2: size (r & 0xFF) + 0x100) and
 * returns 1 from frame 0x1C on. Mode 2 (frame < 0x15 only) draws one
 * func_800CEE20 glow, a func_800D004C ring, three func_800D0728 rings around the
 * func_800CE9D4 world vector and three func_800D3BC8 sprites, each with the
 * D_800F3368 draw block re-seeded; then (every frame) stores the centre in
 * D_8019B678 and the 0x20 draw block.
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
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern B4 D_8018F23C;
extern B4 D_8018F240;
extern GteShortVector D_8019B678;
extern short D_8019B67C;
extern int D_800E27EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern void *D_800F33E0;
extern void *D_800F32D0;
extern unsigned char *D_8009D254;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern unsigned char func_801982A4[];
extern int func_800D3FD8();
extern int func_800D3F64();
extern int func_800CE560();
extern unsigned short *func_800CE610();
extern void func_800CE870();
extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CE9D4();
extern void func_800CEE20();
extern void func_800D004C();
extern void func_800D0728();
extern void func_800D3BC8();

int func_80198914(int mode, unsigned char *a1)
{
    struct {                    /* retail frame 0x30..0x48 */
        GteShortVector v;
        B4 buf1;
        int pad1;
        B4 buf2;
        int pad2;
    } L;
    unsigned short *e;
    int i;
    int r;
    int s6;
    int s4;
    int w;
    int clut;

    L.buf1 = D_8018F23C;
    L.buf2 = D_8018F240;
    switch (mode) {
    case 0:
        SH(a1, 0xA) = 0;
        r = func_800D3FD8();
        SH(a1, 8) = 0;
        func_800D3F64(0x5DF, r);
        func_800D3F64(0x5E0, 0x80);
        func_800CE870(D_8009D254, 0, a1);
        return func_800CE560(P(D_800F33E0, 8), 0x14, 0x3C, func_801982A4);
    case 1:
        if (D_800E27EC < 4) {
            for (i = 0; i < 4; i++) {
                e = func_800CE610(P(D_800F33E0, 8));
                if (e != 0) {
                    e[0] = H(a1, 0);
                    e[1] = H(a1, 2);
                    e[2] = H(a1, 4);
                    r = func_80071A54();
                    e[4] = r % 32 - 16;
                    r = func_80071A54();
                    e[5] = r % 32 - 16;
                    r = func_80071A54();
                    e[6] = r % 32 - 16;
                    e[8] = 1;
                    e[9] = 0;
                }
            }
            for (i = 0; i < 7; i++) {
                e = func_800CE610(P(D_800F33E0, 8));
                if (e != 0) {
                    e[0] = H(a1, 0);
                    e[1] = H(a1, 2);
                    e[2] = H(a1, 4);
                    e[4] = func_80071A54();
                    e[5] = func_80071A54();
                    e[6] = func_80071A54();
                    r = func_80071A54();
                    e[3] = (r & 0x3FF) + 0x5DC;
                    e[8] = 0;
                    e[9] = 0;
                }
            }
            for (i = 0; i < 5; i++) {
                e = func_800CE610(P(D_800F33E0, 8));
                if (e != 0) {
                    e[0] = H(a1, 0);
                    e[1] = H(a1, 2);
                    e[2] = H(a1, 4);
                    e[4] = func_80071A54();
                    e[5] = func_80071A54();
                    e[6] = func_80071A54();
                    r = func_80071A54();
                    e[3] = (r & 0xFF) + 0x100;
                    e[8] = 2;
                    e[9] = 0;
                }
            }
        }
        if (D_800E27EC < 0x1C)
            return 0;
        return 1;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        if (D_800E27EC < 0x15) {
            s6 = func_80077DC4((D_800E27EC << 10) / 20) / 32;
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0x90, w);
            func_800CEE20(a1, 0, 0x2000, 0x2000, 0x98, clut & 0xFFFF, 1, s6, 0);
            func_800D004C(a1, 0x320, 0x320, 0xC, 0, 0x1000, 0x1000, &L.buf1, 0, s6, 1);
            s4 = func_80077CF4((D_800E27EC << 10) / 20);
            s4 = s4 / 2 + 0x1000;
            func_800D0728(a1, 0x226, 0x320, 0x14, 0, s4, s4, 0, &L.buf1, s6 / 2, 1);
            func_800CE9D4(P(D_800F32D0, 8), 0, &L.v);
            s4 = func_80077CF4((D_800E27EC << 10) / 20);
            L.v.w = 1;
            L.v.x += 0x200;
            L.v.y += 0x400;
            func_800D0728(a1, 0x44C, 0x578, 0x14, &L.v, s4, s4, 0, &L.buf2, s6, 1);
            L.v.x -= 0x400;
            func_800D0728(a1, 0x44C, 0x578, 0x14, &L.v, s4, s4, 0, &L.buf2, s6, 1);
            s6 = func_80077DC4((D_800E27EC << 10) / 20) / 32;
            D_800F3368.a68 = 0x40;
            D_800F3368.a6A = 4;
            D_800F3368.a76 = 0x40;
            D_800F3368.a78 = 0x40;
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0xA0, w);
            func_800D3BC8(a1, 0x1000, 0x1000, 0xC, clut & 0xFFFF, 1, s6, 0, 0xC00);
            D_800F3368.a68 = 0x20;
            D_800F3368.a6A = 2;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0xB0, w);
            func_800D3BC8(a1, 0x1000, 0x1000, 0x9A, clut & 0xFFFF, 1, s6, 0, 0xA00);
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            clut = func_80077AA4(0xC0, w);
            func_800D3BC8(a1, 0x1000, 0x1000, 0x9C, clut & 0xFFFF, 1, s6, 0, 0x600);
        }
        D_8019B678.x = H(a1, 0);
        D_8019B678.y = H(a1, 2);
        D_8019B67C = H(a1, 4);
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
