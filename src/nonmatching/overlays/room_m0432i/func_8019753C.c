/* room_m0432i — func_8019753C, blob offset 0x8554, 0xC74 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect-spawner / callback (a0 = command 0/1/2, a1 = effect record, a2 = the
 * parameter block read by command 0), read from the retail disassembly.  Same
 * family as func_80196554 (the callback it registers here is func_8019702C).
 *   cmd 0: clears +0x18/+0x1A/+0x1C/+0x1E and D_80199504, loads the D_800B0E64
 *          resource 0xC54C4704 into D_80199500, flips the first byte of the
 *          D_800F32D0 object's record (1 -> 2) when D_800E2368->+0xD is set,
 *          copies a2 +0/+2/+4 into a1 +8/+0xA/+0xC, puts a1 +0/+2/+4 at
 *          (-0x320, -0x200, 0) and registers func_8019702C with func_800CE560
 *          (its result is the return value).
 *   cmd 1: derives the offset +0x10/+0x12/+0x14 (integrates +8/+0xA/+0xC after
 *          a ramp of +0x1E).  State (+0x18) 0 spawns one particle per frame
 *          for 16 frames then plays two sounds and goes to state 1; state 1
 *          counts to 0x18 spawning spark particles, returns 1 at the end.
 *   cmd 2: loads D_800BCFA4[0] into the GTE, drives the D_80199500 3D effect
 *          (func_800C6xxx) with a scale matrix and draws sprites through
 *          func_800CEE20 / func_800D004C / func_800D0728.  State 0 and state 1
 *          scale by sine / cosine of the frame; every state (and the default
 *          arm) ends D_800F3372 = 0, D_800F3374 = 4 (0x28 stays only inside the
 *          state-0 draw block) and returns 0.
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

extern B4 D_8018F218;
extern B4 D_8018F224;
extern GteShortVector D_8018F1F4;
extern GteShortVector D_8018F1FC;
extern int D_800B0E64;
extern int D_80199500;
extern short D_80199504;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern MATRIX *D_800BCFA4[];
extern unsigned short D_800E11FA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800E27EC;
extern int D_800F3428;
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern unsigned char func_8019702C[];
extern int func_8006E498();
extern int func_8006DF50();
extern int func_80020D50();
extern int func_80020DD0();
extern int func_80071A54();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077A64();
extern int func_80077AA4();
extern void func_80079754();
extern void func_80078CC4();
extern void func_800C6D5C();
extern void func_800C6EC0();
extern void func_800C6ED8();
extern void func_800C6EF8();
extern void func_800C6F4C();
extern void func_800C6FA0();
extern void func_800C71E4();
extern int func_800CE560();
extern unsigned char *func_800CE610();
extern void func_800CEE20();
extern void func_800CFB7C(void *, short, void *);
extern void func_800D004C();
extern void func_800D0728();
extern void func_800D1D24();
extern int func_800D3FD8();

int func_8019753C(int mode, unsigned char *a1, unsigned char *a2)
{
    /* retail frame, sp+0x30 ..: the callees read it through stack pointers */
    struct {
        B4 b30; B4 g34; B4 b38; B4 g3c;
        GteShortVector v;           /* sp+0x40 */
        GteShortVector v48;         /* sp+0x48 */
        GteShortVector b50;         /* sp+0x50 */
        GteShortVector b58;         /* sp+0x58 */
        MATRIX M;                   /* sp+0x60 (t at +0x74) */
        int sc[3]; int g8c;         /* sp+0x80 */
        MATRIX M1;                  /* sp+0x90 (t at +0xA4) */
        int sc1[3];                 /* sp+0xB0 */
    } fr;
    MATRIX *mm;
    int *scp;
    unsigned char *p;
    unsigned char **q;
    unsigned char *r;
    unsigned char *t;
    int n, s4, s5, s0, a, clut, i, sh, hz;

    fr.b30 = D_8018F218;
    fr.b38 = D_8018F224;
    fr.b50 = D_8018F1F4;
    fr.b58 = D_8018F1FC;

    if (mode == 1) {
        sh = (0x10 - SH(a1, 0x1E)) * 5;
        func_800CFB7C(a1, (int)(-((unsigned)sh << 22)) >> 16, a1 + 0x10);
        H(a1, 0x10) += H(a1, 8);
        H(a1, 0x12) += H(a1, 0xA);
        H(a1, 0x14) += H(a1, 0xC);
        func_800CFB7C(a1, 0x140, &fr.v);
        fr.v.x += H(a1, 0x10);
        fr.v.y += H(a1, 0x12);
        fr.v.z += H(a1, 0x14);
        switch (SH(a1, 0x18)) {
        case 0:
            H(a1, 0x1A)++;
            H(a1, 0x1E)++;
            p = func_800CE610(*(int *)(D_800F33E0 + 8));
            if (p) {
                H(p, 0) = fr.v.x;
                H(p, 2) = fr.v.y;
                H(p, 4) = fr.v.z;
                H(p, 6) = (func_80071A54() & 0xFF) + 0x180;
                H(p, 8) = H(a1, 0);
                H(p, 0xA) = H(a1, 2);
                H(p, 0xC) = H(a1, 4);
                H(p, 0xE) = func_80071A54();
                H(p, 0x10) = 0;
                H(p, 0x12) = 0;
            }
            if (SH(a1, 0x1A) < 0x10)
                return 0;
            SH(a1, 0x18) = 1;
            SH(a1, 0x1A) = 0;
            if (D_800B0E64 == 0)
                return 0;
            n = func_800D3FD8();
            func_8006DF50(D_800B0E64, 0x5D8, n, 0x80, 0x7F);
            if (D_800B0E64 == 0)
                return 0;
            func_8006DF50(D_800B0E64, 0x5D9, 0x80, 0x80, 0x7F);
            return 0;
        case 1:
            H(a1, 0x1A)++;
            s5 = (SH(a1, 0x1A) << 10) / 24;
            func_80020D50(0x2AAAAAAB);
            if (SH(a1, 0x1A) < 9)
                func_800D1D24(2, 8, SH(a1, 0x1A));
            H(a1, 0x10) = H(a1, 8);
            H(a1, 0x12) = H(a1, 0xA);
            H(a1, 0x14) = H(a1, 0xC);
            if (D_800E27EC & 1) {
                n = func_80077DC4(s5);
                H(a1, 0x12) = H(a1, 0x12) + n / 512;
            } else {
                n = func_80077DC4(s5);
                H(a1, 0x12) = H(a1, 0x12) - n / 512;
            }
            if (SH(a1, 0x1A) < 0xD) {
                p = func_800CE610(*(int *)(D_800F33E0 + 8));
                if (p) {
                    H(p, 0) = H(a1, 8);
                    H(p, 2) = H(a1, 0xA);
                    H(p, 4) = H(a1, 0xC);
                    H(p, 6) = (func_80071A54() & 0x3FF) + 0x7D0;
                    H(p, 8) = 0;
                    H(p, 0xA) = func_80071A54();
                    H(p, 0xC) = 0;
                    H(p, 0x10) = 1;
                    H(p, 0x12) = 0;
                }
            }
            if ((unsigned)(H(a1, 0x1A) - 4) < 5) {
                for (i = 0; i < 2; i++) {
                    p = func_800CE610(*(int *)(D_800F33E0 + 8));
                    if (p) {
                        H(p, 0) = H(a1, 8);
                        H(p, 2) = H(a1, 0xA);
                        H(p, 4) = H(a1, 0xC);
                        n = func_80071A54();
                        H(p, 8) = n % 768 + 0x100;
                        H(p, 0xA) = func_80071A54();
                        H(p, 0xC) = func_80071A54();
                        H(p, 0x10) = 2;
                        H(p, 0x12) = 0;
                    }
                }
            }
            if (SH(a1, 0x1A) < 0x18)
                return 0;
            func_80020DD0();
            return 1;
        }
        return 0;
    }
    if (mode == 0) {
        H(a1, 0x18) = 0;
        H(a1, 0x1A) = 0;
        H(a1, 0x1C) = 0;
        H(a1, 0x1E) = 0;
        D_80199504 = 0;
        D_80199500 = func_8006E498(D_800B0E64, 0xC54C4704, a2);
        func_800C6D5C(D_80199500, 0, 0);
        if (D_800E2368[0xD] != 0) {
            q = *(unsigned char ***)(D_800F32D0 + 8);
            if (q) {
                r = *q;
                if (r) {
                    t = *(unsigned char **)(r + 0x18);
                    if (*t == 1)
                        *t = 2;
                }
            }
        }
        H(a1, 8) = H(a2, 0);
        H(a1, 0xA) = H(a2, 2);
        hz = H(a2, 4);
        H(a1, 0) = -0x320;
        H(a1, 2) = -0x200;
        H(a1, 4) = 0;
        H(a1, 0xC) = hz;
        return func_800CE560(*(int *)(D_800F33E0 + 8), 0x14, 0x20, func_8019702C);
    }
    if (mode != 2)
        return 0;

    gte_SetRotMatrix(D_800BCFA4[0]);
    gte_SetTransMatrix(D_800BCFA4[0]);
    func_800CFB7C(a1, 0x140, &fr.v);
    fr.v.x += H(a1, 0x10);
    fr.v.y += H(a1, 0x12);
    fr.v.z += H(a1, 0x14);

    switch (SH(a1, 0x18)) {
    case 0:
        s5 = SH(a1, 0x1A) << 6;
        n = func_80077DC4(s5);
        s4 = (n / 2 + 0x800) / 32;
        break;
    case 1:
        s5 = (SH(a1, 0x1A) << 10) / 24;
        n = func_80077DC4(s5);
        s4 = n / 32;
        break;
    default:
        goto done;
    }
    if (D_800E27EC & 1)
        s4 = (s4 * 3) / 4;

    D_800F3368.a6C = 3;
    D_800F3368.a6E = 1;
    D_800F3368.a70 = D_800E2850[D_800E11FA];
    s0 = func_80077A64(0, 1, 0, 0);
    s0 = (D_800E2850[D_800E11FA] | s0) & 0xFFFF;
    a = D_800E1204[D_800F3368.a6C];
    if (D_800F3368.a6C == 4 && D_800F3428 != 0)
        a += 7;
    else
        a += 3;
    clut = func_80077AA4(0, a);
    func_800C6EC0(s0, clut & 0xFFFF);
    func_800C6ED8(1);
    fr.v48.x = H(a1, 0);
    fr.v48.y = H(a1, 2);
    fr.v48.z = H(a1, 4);
    fr.v48.x += 0x800;
    /* state 0 builds its matrix at sp+0x60 / scale at sp+0x80, state 1 at sp+0x90 / 0xB0 */
    if (SH(a1, 0x18) == 0) { mm = &fr.M; scp = fr.sc; } else { mm = &fr.M1; scp = fr.sc1; }
    func_80079754(&fr.v48, mm);
    mm->t[0] = SH(a1, 0x10);
    mm->t[1] = SH(a1, 0x12);
    scp[0] = 0x2AA;
    scp[1] = 0x2AA;
    scp[2] = 0x2AA;
    mm->t[2] = SH(a1, 0x14);
    func_80078CC4(mm, scp);
    func_800C6EF8(D_80199500);
    func_800C6FA0(D_80199500, s4 & 0xFFFF);
    func_800C71E4(D_80199500, mm);

    if (SH(a1, 0x18) == 0) {
        func_800C6F4C(D_80199500);
        D_800F3368.a68 = 0x40;
        D_800F3368.a6A = 4;
        D_800F3368.a76 = 0x40;
        D_800F3368.a78 = 0x40;
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 1;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0x28;
        D_800F3368.a70 = D_800E2850[D_800E11FA];
        fr.b50.z = SH(a1, 0x1A) << 5;
        clut = func_80077AA4(0, D_800E120A + 3);
        func_800CEE20(&fr.v, &fr.b50, 0x1000, 0x1000, 4, clut & 0xFFFF, 1, s4 / 2, 0);
        s0 = func_80077CF4(s5) + 0x1000;
        func_800D004C(&fr.v, 0x44C, 0x5DC, 0x20, 0, s0, s0, &fr.b30, 0, s4, 1);
    } else {
        p = a1 + 8;
        func_800C6F4C(D_80199500);
        s0 = func_80077DC4(s5) + 0x1000;
        fr.b50.z = SH(a1, 0x1A) << 5;
        func_800D004C(p, 0x3E8, 0x32, 8, &fr.b50, s0, s0, &fr.b38, 0, s4, 1);
        s0 = func_80077CF4(s5);
        fr.b58.z = -(SH(a1, 0x1A) << 5);
        func_800D0728(p, 0x7D0, 0xFA0, 0x10, &fr.b58, s0, s0, 0, &fr.b30, s4, 1);
    }

done:
    D_800F3368.a72 = 0;
    D_800F3368.a74 = 4;
    return 0;
}
