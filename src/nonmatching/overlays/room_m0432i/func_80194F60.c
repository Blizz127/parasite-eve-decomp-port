/* room_m0432i — func_80194F60, blob offset 0x5F78, 0x86C bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-object callback, modes 0/1/2 on a point record a1, read from the
 * retail disassembly (sibling of room_m0432i func_8019485C and room_m0429i
 * func_801960C4/func_80196CCC).
 * mode 0: plays sound 0x5DC (func_800D3FD8() result) and 0x5DD, reads the world
 *   position of D_8009D254 into *a1 (func_800CE870), flips the byte at
 *   D_800F32D0->8->0->0x18 from 1 to 2 when D_800E2368->0xD is set, and arms
 *   func_8019485C through func_800CE560(D_800F33E0->8, 0x14, 0x3C, ...).
 * mode 1: frame counter D_800E27EC. func_80020D50() below 0x1C; two state-0
 *   sparks (+-32 velocity) up to 0x10; a state-1 spark (+-20/+-16/+-20) up to
 *   0x20 and, on odd frames, a state-2 spark (+-24); func_80020DD0() at 0x1C;
 *   at 0x1D (with D_800E2368->0xD set and the top flag bits of
 *   ***(D_800F32D0+8) == 0x01000000) sets bit 0x4000 in *D_8009D254->0x4C and
 *   rewrites/sets bits in the record word; returns 2 from frame 0x20.
 * mode 2: loads the camera matrix D_800BCFA4[0] into the GTE, fills the
 *   D_800F3368 draw block (size 0x20) and, up to frame 0x20, draws a
 *   func_800CEE20 sprite and func_800D004C / func_800D0728 rings sized from
 *   func_80077CF4; finally re-seeds a72 = 0 / a74 = 8.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros.
 * Retail frame: sp+0x30 = D_8018F1FC (8 bytes), +0x38 vector, +0x40 =
 * D_8018F210 (4 bytes), +0x48 colour (written by func_800CF3AC). */
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

extern B8 D_8018F1FC;
extern B4 D_8018F210;
extern unsigned char D_80199388[];
extern unsigned char func_8019485C[];
extern short D_800942EC;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800E27EC;
extern unsigned char *D_800E2368;
extern void *D_8009D254;
extern void *D_800F32D0;
extern void *D_800F33E0;
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80020D50();
extern int func_80020DD0();
extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077AA4();
extern int func_800CE560();
extern unsigned short *func_800CE610();
extern void func_800CE870();
extern void func_800CEE20();
extern void func_800CF3AC();
extern void func_800D004C();
extern void func_800D0728();
extern int func_800D3F64();
extern int func_800D3FD8();

int func_80194F60(int mode, unsigned char *a1)
{
    struct {                    /* retail frame 0x30..0x50 */
        B8 b30;
        GteShortVector v;
        B4 b40;
        int pad44;
        unsigned char b48[8];
    } L;
    unsigned short *e;
    unsigned char *p;
    int i;
    int s4;
    int s5;
    int t;
    int w;
    int clut;

    L.b30 = D_8018F1FC;
    L.b40 = D_8018F210;
    switch (mode) {
    case 0:
        func_800D3F64(0x5DC, func_800D3FD8());
        func_800D3F64(0x5DD, 0x80);
        func_800CE870(D_8009D254, 0, a1);
        if (D_800E2368[0xD] != 0) {
            p = P(D_800F32D0, 8);
            if (p != 0) {
                p = *(unsigned char **)p;
                if (p != 0) {
                    p = *(unsigned char **)(p + 0x18);
                    if (p[0] == 1)
                        p[0] = 2;
                }
            }
        }
        return func_800CE560(P(D_800F33E0, 8), 0x14, 0x3C, func_8019485C);
    case 1:
        if (D_800E27EC < 0x1C)
            func_80020D50();
        if (D_800E27EC < 0x11) {
            for (i = 0; i < 2; i++) {
                e = func_800CE610(P(D_800F33E0, 8));
                if (e != 0) {
                    e[0] = H(a1, 0);
                    e[1] = H(a1, 2);
                    e[2] = H(a1, 4);
                    e[4] = func_80071A54() % 64 - 0x20;
                    e[5] = func_80071A54() % 64 - 0x20;
                    e[6] = func_80071A54() % 64 - 0x20;
                    e[8] = 0;
                    e[9] = 0;
                }
            }
        }
        if (D_800E27EC < 0x21) {
            e = func_800CE610(P(D_800F33E0, 8));
            if (e != 0) {
                e[0] = H(a1, 0);
                e[1] = H(a1, 2);
                e[2] = H(a1, 4);
                e[4] = func_80071A54() % 40 - 0x14;
                e[5] = func_80071A54() % 32 - 0x10;
                e[8] = 1;
                e[9] = 0;
                e[6] = func_80071A54() % 40 - 0x14;
            }
            if ((D_800E27EC & 1) != 0) {
                e = func_800CE610(P(D_800F33E0, 8));
                if (e != 0) {
                    e[0] = H(a1, 0);
                    e[1] = H(a1, 2);
                    e[2] = H(a1, 4);
                    e[4] = func_80071A54() % 48 - 0x18;
                    e[5] = func_80071A54() % 48 - 0x18;
                    e[8] = 2;
                    e[9] = 0;
                    e[6] = func_80071A54() % 48 - 0x18;
                }
            }
        }
        if (D_800E27EC == 0x1C)
            func_80020DD0();
        if (D_800E27EC == 0x1D && D_800E2368[0xD] != 0
            && (**(unsigned int **)P(D_800F32D0, 8) & 0x3F000000) == 0x01000000) {
            unsigned int *q;
            p = *(unsigned char **)D_8009D254;
            *(unsigned int *)(p + 0x4C) |= 0x4000;
            q = *(unsigned int **)P(D_800F32D0, 8);
            *q = (*q & 0xC0FFFFFF) | 0x19000000;
            q = *(unsigned int **)P(D_800F32D0, 8);
            *q |= 0x80000000;
        }
        if (D_800E27EC < 0x20)
            return 0;
        return 2;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11FA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        D_800F3368.a72 = 5;
        D_800F3368.a74 = 0x20;
        if (D_800E27EC < 0x21) {
            s5 = func_80077CF4(D_800E27EC << 6) / 32;
            if ((D_800E27EC & 1) != 0)
                s5 = s5 * 15 / 16;
            s4 = func_80077CF4(D_800E27EC << 5) / 2 + 0x800;
            func_800CE870(P(D_800F32D0, 8), 0, &L.v);
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 0xB;
            else
                w += 7;
            clut = func_80077AA4(0, w);
            t = s4 * 3 / 2;
            func_800CEE20(&L.v, 0, t, t, 0x46, clut & 0xFFFF, 1, s5, 0);
            func_800D004C(&L.v, 0x320, 0x258, 0x10, 0, 0x1000, 0x1000, &L.b40, 0, s5, 1);
            s4 = func_80077CF4(D_800E27EC << 5);
            L.v.x = H(a1, 0);
            L.v.y = H(a1, 2);
            L.v.y = D_800942EC;
            L.v.z = H(a1, 4);
            func_800CF3AC(D_80199388, L.b48, D_800E27EC);
            func_800D0728(&L.v, 0x258, 0x384, 0x14, &L.b30, s4, s4, 0, L.b48, s5 / 2, 1);
        }
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 8;
        return 0;
    }
    return 0;
}
