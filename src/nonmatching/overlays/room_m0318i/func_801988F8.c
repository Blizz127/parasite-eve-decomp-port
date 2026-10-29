/* room_m0318i — func_801988F8, blob offset 0x9910, 0x920 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect main for the sprite/model burst of this room, read from the retail
 * disassembly (7E64.s). a0 = mode, a1 = effect record (+0/+2/+4 position,
 * +8/+A/+C counters, written here).
 * mode 0: clear a1 +8/+A/+C; if D_800B0E64 is set, play sound 0x5ED (volume
 *         func_800D3FD8()) and, if it is still set, 0x5EE (func_8006DF50);
 *         load the model 0xC5509704 into D_80199944 (func_8006E498 +
 *         func_800C6D5C); when D_800E2368[0xD] is set flip the state byte at
 *         D_800F32D0->+8->+0->+0x18 from 1 to 2; run func_800CE9D4 and start
 *         the 0x14-slot pool with the particle callback func_80198268
 *         (returns func_800CE560's result).
 * mode 1: a1 +A is the frame (a1 +C = sin((f << 10) / 24) / 32); func_800CE870
 *         fills the stack vector with the target position. For f < 0x19 it
 *         spawns three pool particles at that position: a scattered spark
 *         (+0..+6 jittered by 6-bit randoms, a1 position at +8..+C, type 1),
 *         on odd frames a wide one (9-bit jitter and %32 velocities, type 0)
 *         and a small one (5-bit jitter, type 2). Then, if the position is
 *         within 0x19A of the player, D_800E2368[0xD] is set and the hit
 *         object word has the 0x01000000 state, it flags the D_8009D254
 *         record (+0x4C |= 0x4000) and moves the hit word to state 0x19
 *         with the top bit set. Returns 2 once f >= 0x18, else 0.
 * mode 2: GTE rotation/translation from D_800BCFA4[0]; f >= 0x19 only resets
 *         D_800F3372/D_800F3374 (4). Otherwise it draws the 0x384 x 0x320
 *         ring (func_800D004C) with the D_80199890 colour curve and a model
 *         at the target position + a1 offset with a sin/cos-sized scale
 *         (func_80078CC4) and frame a1 +C (x 3/4 on odd frames), tpage and
 *         CLUT from the D_800E2850 / D_800E1204 tables. Returns 0.
 * The 4th halfword of the stack angle vector (sp+0x46) is never written
 * (the vector is passed to func_80079754).
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
#define P(o, x) (*(unsigned char **)((char *)(o) + (x)))

extern int D_800B0E64;
extern int D_80199944;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern unsigned char *D_8009D254;
extern int D_800E27EC;
extern int D_800F3428;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern unsigned char D_80199890[];
extern MATRIX *D_800BCFA4[];
extern short D_800F336C;
extern short D_800F336E;
extern short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern unsigned char func_80198268[];

extern int func_800D3FD8();
extern int func_8006DF50();
extern int func_8006E498();
extern void func_800C6D5C();
extern void func_800CE9D4();
extern int func_800CE560();
extern void func_800CE870();
extern int func_80077DC4();
extern int func_80077CF4();
extern unsigned char *func_800CE610();
extern int func_80071A54();
extern int func_800C6B90();
extern void func_800CF3AC();
extern void func_800D004C();
extern void func_800CFB7C(void *, short, void *);
extern int func_80077A64();
extern int func_80077AA4();
extern void func_800C6EC0();
extern void func_800C6ED8();
extern void func_80079754();
extern void func_80078CC4();
extern void func_800C6EF8();
extern void func_800C6FA0();
extern void func_800C71E4();
extern void func_800C6F4C();

int func_801988F8(int mode, unsigned char *a1)
{
    GteShortVector v;      /* sp+0x30 */
    GteShortVector w;      /* sp+0x38 */
    GteShortVector ang;    /* sp+0x40 */
    B4 buf;                /* sp+0x48 */
    MATRIX mat;            /* sp+0x50 */
    int sc[3];             /* sp+0x70 */
    unsigned char *p;
    unsigned char *x;
    unsigned char *q;
    int r, t, k, c;
    int s3, s4, s5, s0;

    switch (mode) {
    case 0:
        H(a1, 8) = 0;
        H(a1, 0xA) = 0;
        H(a1, 0xC) = 0;
        if (D_800B0E64 != 0) {
            r = func_800D3FD8();
            func_8006DF50(D_800B0E64, 0x5ED, r, 0x80, 0x7F);
            if (D_800B0E64 != 0)
                func_8006DF50(D_800B0E64, 0x5EE, 0x80, 0x80, 0x7F);
        }
        D_80199944 = func_8006E498(D_800B0E64, 0xC5509704);
        func_800C6D5C(D_80199944, 0, 0);
        if (D_800E2368[0xD] != 0) {
            p = P(D_800F32D0, 8);
            if (p != 0) {
                p = P(p, 0);
                if (p != 0) {
                    p = P(p, 0x18);
                    if (p[0] == 1)
                        p[0] = 2;
                }
            }
        }
        func_800CE9D4(P(D_800F32D0, 8), 0, a1);
        return func_800CE560(W(D_800F33E0, 8), 0x14, 0x28, func_80198268);
    case 1:
        H(a1, 0xA)++;
        func_800CE870(P(D_800F32D0, 8), 0, &v);
        t = SH(a1, 0xA) << 10;
        r = func_80077DC4(t / 24);
        SH(a1, 0xC) = r / 32;
        if (SH(a1, 0xA) < 0x19) {
            p = func_800CE610(W(D_800F33E0, 8));
            if (p != 0) {
                SH(p, 0) = v.x;
                SH(p, 2) = v.y;
                SH(p, 4) = v.z;
                r = func_80071A54();
                H(p, 0) = H(p, 0) - 0x20 + (r & 0x3F);
                r = func_80071A54();
                H(p, 2) = H(p, 2) - 0x20 + (r & 0x3F);
                r = func_80071A54();
                H(p, 4) = H(p, 4) - 0x20 + (r & 0x3F);
                r = func_80071A54();
                H(p, 6) = (r & 0x1FF) + 0x200;
                H(p, 8) = H(a1, 0);
                H(p, 0xA) = H(a1, 2);
                H(p, 0xC) = H(a1, 4);
                r = func_80071A54();
                H(p, 0xE) = r;
                H(p, 0x10) = 1;
                H(p, 0x12) = 0;
            }
            if (H(a1, 0xA) & 1) {
                p = func_800CE610(W(D_800F33E0, 8));
                if (p != 0) {
                    SH(p, 0) = v.x;
                    SH(p, 2) = v.y;
                    SH(p, 4) = v.z;
                    r = func_80071A54();
                    H(p, 2) = H(p, 2) + 0x100 - (r & 0x1FF);
                    r = func_80071A54();
                    H(p, 0) = H(p, 0) - 0x100 + (r & 0x1FF);
                    r = func_80071A54();
                    H(p, 4) = H(p, 4) - 0x100 + (r & 0x1FF);
                    r = func_80071A54();
                    H(p, 8) = r % 32 - 0x10;
                    r = func_80071A54();
                    H(p, 0xA) = r % 32 - 0x10;
                    r = func_80071A54();
                    H(p, 0xC) = r % 32 - 0x10;
                    H(p, 0x10) = 0;
                    H(p, 0x12) = 0;
                }
            }
            p = func_800CE610(W(D_800F33E0, 8));
            if (p != 0) {
                SH(p, 0) = v.x;
                SH(p, 2) = v.y;
                SH(p, 4) = v.z;
                r = func_80071A54();
                H(p, 0) = H(p, 0) - 0x10 + (r & 0x1F);
                r = func_80071A54();
                H(p, 2) = H(p, 2) - 0x10 + (r & 0x1F);
                r = func_80071A54();
                H(p, 0x10) = 2;
                H(p, 0x12) = 0;
                H(p, 4) = H(p, 4) - 0x10 + (r & 0x1F);
            }
        }
        if (func_800C6B90(&v, 0x19A)) {
            if (D_800E2368[0xD] != 0) {
                x = D_800F32D0;
                if ((W(P(P(x, 8), 0), 0) & 0x3F000000) == 0x01000000) {
                    q = P(D_8009D254, 0);
                    W(q, 0x4C) |= 0x4000;
                    q = P(P(x, 8), 0);
                    W(q, 0) = (W(q, 0) & 0xC0FFFFFF) | 0x19000000;
                    q = P(P(x, 8), 0);
                    W(q, 0) |= 0x80000000;
                }
            }
        }
        if (SH(a1, 0xA) < 0x18)
            return 0;
        return 2;
    case 2: {
        MATRIX *m = D_800BCFA4[0];
        gte_SetRotMatrix(m);
        gte_SetTransMatrix(m);
        func_800CE870(P(D_800F32D0, 8), 0, &v);
        if (SH(a1, 0xA) >= 0x19) {
            D_800F3372 = 0;
            D_800F3374 = 4;
            return 0;
        }
        s3 = SH(a1, 0xC);
        if (D_800E27EC & 1)
            s3 = (s3 * 3) / 4;
        D_800F3374 = 0x28;
        func_800CF3AC(D_80199890, &buf, (SH(a1, 0xA) << 5) / 24);
        func_800D004C(&v, 0x384, 0x320, 0xC, 0, 0x1000, 0x1000, &buf, 0, s3, 1);
        s0 = (SH(a1, 0xA) << 10) / 24;
        r = func_80077CF4(s0);
        s5 = r / 2 + 0x800;
        r = func_80077CF4(s0);
        s4 = r / 4 + 0xC00;
        func_800CFB7C(a1, 0xC8, &w);
        w.x = w.x + v.x;
        w.y = w.y + v.y;
        w.z = w.z + v.z;
        ang.x = H(a1, 0);
        ang.y = H(a1, 2) + 0x800;
        ang.z = H(a1, 4) + SH(a1, 0xA) * 44;
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3374 = 4;
        D_800F3370 = D_800E2850[D_800E11EA];
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3370 = D_800E2850[D_800E11EA];
        r = func_80077A64(0, 1, 0, 0);
        k = (D_800E2850[D_800E11EA] | r) & 0xFFFF;
        c = D_800E1204[(unsigned short)D_800F336C];
        if ((unsigned short)D_800F336C == 4 && D_800F3428 != 0)
            c += 4;
        r = func_80077AA4(0x40, c);
        func_800C6EC0(k, r & 0xFFFF);
        func_800C6ED8(1);
        func_80079754(&ang, &mat);
        sc[0] = s4;
        sc[1] = s4;
        sc[2] = s5;
        mat.t[0] = w.x;
        mat.t[1] = w.y;
        mat.t[2] = w.z;
        func_80078CC4(&mat, sc);
        func_800C6EF8(D_80199944);
        func_800C6FA0(D_80199944, s3 & 0xFFFF);
        func_800C71E4(D_80199944, &mat);
        func_800C6F4C(D_80199944);
        D_800F3372 = 0;
        D_800F3374 = 4;
        return 0;
    }
    default:
        return 0;
    }
}
