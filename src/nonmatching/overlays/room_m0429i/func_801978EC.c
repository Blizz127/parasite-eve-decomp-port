/* room_m0429i — func_801978EC, blob offset 0x8904, 0x9B8 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback, modes 0/1/2 on a record a1 (state +8, frame +0xA, scale +0xC),
 * read from the retail disassembly. Mode 0 clears state/frame/scale, arms the
 * D_800F32D0 object byte, plays two sounds via D_800B0E64, registers the
 * func_801972EC child through func_800CE560. Mode 1 steps the record: state 0
 * (16 frames) ramps +0xC by func_80077CF4, state 1 (32 frames) spawns
 * particles from the func_800CE610 pool, state 2 (24 frames) fades +0xC with
 * func_80077DC4 and, on frame 1, flips the D_800F32D0 object's flag bits.
 * Mode 2 loads the GTE rotation/translation from D_800BCFA4[0], sets the
 * D_800F3368 draw block and draws two func_800CEE20 sprites plus the
 * func_800D004C / func_800D0728 glows when the +0xC scale is non-zero.
 * Retail frame 0x30..0x57 (buf, then the four short vectors at 0x38/0x40/0x48/
 * 0x50) is reproduced as one struct so the stack-pointer arguments carry the
 * same neighbouring bytes.
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

extern B4 D_8018F238;
extern short D_800942EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E11FA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800E27EC;
extern int D_800F3428;
extern int D_800B0E64;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern int **D_8009D254;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern unsigned char func_801972EC[];
extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern int func_80020D50();
extern int func_80020DD0();
extern int func_8006DF50();
extern int func_800D3FD8();
extern int func_800CE560();
extern int func_800CE870();
extern unsigned char *func_800CE610();
extern void func_800CEE20();
extern void func_800D004C();
extern void func_800D0728();

int func_801978EC(int mode, unsigned char *a1)
{
    struct {                    /* retail frame 0x30..0x57 */
        B4 buf; int g34;
        GteShortVector v38;
        GteShortVector v40;
        GteShortVector v48;
        GteShortVector v50;
    } L;
    unsigned char *p;
    unsigned char *q;
    unsigned char *s;
    int *w;
    int s4;
    int r;
    int f;
    int t;
    int clut;
    int a0;

    L.buf = D_8018F238;
    switch (mode) {
    case 0:
        SH(a1, 8) = 0;
        SH(a1, 0xA) = 0;
        SH(a1, 0xC) = 0;
        if (D_800E2368[0xD] != 0) {
            p = (unsigned char *)W(D_800F32D0, 8);
            if (p != 0) {
                q = (unsigned char *)W(p, 0);
                if (q != 0) {
                    s = (unsigned char *)W(q, 0x18);
                    if (*s == 1)
                        *s = 2;
                }
            }
        }
        if (D_800B0E64 != 0) {
            t = func_800D3FD8(1, 1);
            func_8006DF50(D_800B0E64, 0x5E5, t, 0x80, 0x7F);
            if (D_800B0E64 != 0)
                func_8006DF50(D_800B0E64, 0x5E6, 0x80, 0x80, 0x7F);
        }
        func_800CE870(D_8009D254, 1, a1);
        SH(a1, 2) = D_800942EC;
        return func_800CE560(W(D_800F33E0, 8), 0x14, 0x28, func_801972EC);
    case 1:
        if (SH(a1, 8) < 2)
            func_80020D50();
        switch (SH(a1, 8)) {
        case 0:
            f = (unsigned short)(H(a1, 0xA) + 1);
            H(a1, 0xA) = f;
            t = func_80077CF4((short)f << 6);
            SH(a1, 0xC) = t / 32;
            if (SH(a1, 0xA) < 0x10)
                return 0;
            SH(a1, 8) = 1;
            H(a1, 0xA) = 0;
            return 0;
        case 1:
            H(a1, 0xA)++;
            p = func_800CE610(W(D_800F33E0, 8));
            if (p != 0) {
                H(p, 0) = H(a1, 0);
                H(p, 2) = H(a1, 2);
                H(p, 4) = H(a1, 4);
                r = func_80071A54();
                SH(p, 8) = r % 50 - 0x19;
                r = func_80071A54();
                SH(p, 0xA) = (r / 32) * 32 - r;
                r = func_80071A54();
                H(p, 0x10) = 1;
                H(p, 0x12) = 0;
                SH(p, 0xC) = r % 50 - 0x19;
            }
            if (H(a1, 0xA) & 1) {
                p = func_800CE610(W(D_800F33E0, 8));
                if (p != 0) {
                    H(p, 0) = H(a1, 0);
                    H(p, 2) = H(a1, 2);
                    H(p, 4) = H(a1, 4);
                    r = func_80071A54();
                    H(p, 2) = H(p, 2) - (r & 0x1FF);
                    r = func_80071A54();
                    H(p, 0) = H(p, 0) - 0x40 + (r & 0x7F);
                    r = func_80071A54();
                    H(p, 4) = H(p, 4) - 0x40 + (r & 0x7F);
                    r = func_80071A54();
                    SH(p, 8) = r % 24 - 0xC;
                    r = func_80071A54();
                    SH(p, 0xA) = r % 24 - 0xC;
                    r = func_80071A54();
                    H(p, 0x10) = 0;
                    H(p, 0x12) = 0;
                    SH(p, 0xC) = r % 24 - 0xC;
                }
            }
            if (SH(a1, 0xA) % 6 == 0) {
                p = func_800CE610(W(D_800F33E0, 8));
                if (p != 0) {
                    H(p, 0) = H(a1, 0);
                    H(p, 2) = H(a1, 2);
                    H(p, 4) = H(a1, 4);
                    H(p, 0x10) = 2;
                    H(p, 0x12) = 0;
                }
            }
            if (SH(a1, 0xA) < 0x20)
                return 0;
            func_80020DD0();
            SH(a1, 8) = 2;
            H(a1, 0xA) = 0;
            return 0;
        case 2:
            f = (unsigned short)(H(a1, 0xA) + 1);
            H(a1, 0xA) = f;
            t = func_80077DC4(((short)f << 10) / 24);
            SH(a1, 0xC) = t / 32;
            if (SH(a1, 0xA) == 1 && D_800E2368[0xD] != 0 &&
                (***(int ***)(D_800F32D0 + 8) & 0x3F000000) == 0x01000000) {
                w = *D_8009D254;
                w[0x4C / 4] |= 0x4000;
                w = *(int **)W(D_800F32D0, 8);
                *w = (*w & 0xC0FFFFFF) | 0x19000000;
                w = *(int **)W(D_800F32D0, 8);
                *w |= 0x80000000;
            }
            if (SH(a1, 0xA) < 0x18)
                return 0;
            return 1;
        }
        return 0;
    case 2:
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        D_800F3368.a68 = 0x40;
        D_800F3368.a6A = 4;
        D_800F3368.a76 = 0x40;
        D_800F3368.a78 = 0x40;
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        D_800F3368.a72 = 0;
        D_800F3368.a70 = D_800E2850[D_800E11FA];
        if (SH(a1, 0xC) != 0) {
            s4 = SH(a1, 0xC);
            if (H(a1, 0xA) & 1)
                s4 = s4 * 15 / 16;
            a0 = D_800E27EC;
            if (a0 & 1)
                s4 = s4 * 2 / 3;
            L.v40.x = 0x400;
            L.v40.z = a0 << 3;
            D_800F3368.a74 = 0;
            L.v40.y = 0;
            L.v40.w = 1;
            L.v38.x = H(a1, 0);
            L.v38.y = D_800942EC;
            L.v38.z = H(a1, 4);
            t = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                t += 6;
            else
                t += 2;
            clut = func_80077AA4(0, t);
            func_800CEE20(&L.v38, &L.v40, 0x1000, 0x1000, 0, clut & 0xFFFF, 1, s4, 0);
            D_800F3368.a6C = 3;
            L.v50.x = 0x400;
            D_800F3368.a6E = 0;
            L.v50.y = 0;
            L.v50.z = D_800E27EC << 5;
            L.v50.w = 1;
            D_800F3368.a70 = D_800E2850[D_800E11EA];
            L.v48.x = H(a1, 0);
            L.v48.y = D_800942EC;
            L.v48.z = H(a1, 4);
            clut = func_80077AA4(0x10, D_800E120A);
            func_800CEE20(&L.v48, &L.v50, 0x1800, 0x1800, 0x40, clut & 0xFFFF, 1, s4, 0);
            D_800F3368.a74 = 0x28;
            func_800D004C(a1, 0x190, 0x190, 0x10, 0, 0x1000, 0x1000, &L.buf, 0, s4, 1);
            func_800D0728(a1, 0x1F4, 0x258, 0x18, 0, 0x1000, 0x1000, 0, &L.buf, s4, 3);
        }
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 4;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        return 0;
    }
    return 0;
}
