/* func_800D5290 -- EXE, VRAM 0x800D5290, file offset 0xC5A90, 0x608 bytes.
 * NON_MATCHING behaviour C (proven by the e21 differential; not a byte match).
 * Three-mode effect callback (a0 = mode, a1 = effect record); the EXE twin of
 * the room callbacks room_m0429i func_801982A4 / room_m0269i func_8019070C:
 *   mode 0: seed the record (rand into +8, +0/+2/+4 = halfwords 0x268/0x26A/
 *           0x26C of the object at D_800F32D0->+8), then
 *           func_800CE560(D_800F33E0->+8, 0x10, 0xC, func_800D5010); its result
 *           is the return value.
 *   mode 1: while D_800E27EC < 6 spawn two particles from
 *           func_800CE610(D_800F33E0->+8) (position = record +0/+2/+4, speed
 *           from sin/cos(record +8) * (rand & 15) + 0x2B / 4096, random drift
 *           words, record +8 += 0x955); returns D_800E27EC >= 0x18.
 *   mode 2: set the camera matrix *D_800BCFA4, set up the D_800F3368 draw block,
 *           draw the glow with func_800CEE20 around a stack vector
 *           {0, 0, f << 4, 0} (f = D_800E27EC), and while D_800E27EC < 9 a
 *           func_800D004C sprite; for 4 <= f <= 16 draw func_800D0728 through
 *           a stack vector {record x, D_800942EC, record z}; then restore the
 *           draw block (D_800F3374 = 0x40). Returns 0.
 * Retail copies the 4 bytes at D_800C22BC to a stack buffer (sp+0x40) passed to
 * func_800D004C / func_800D0728; the vector at sp+0x38 has its fourth halfword
 * (sp+0x3E) never written -- retail passes it as-is.
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

extern B4 D_800C22BC;
extern int *D_800F32D0;
extern int *D_800F33E0;
extern int D_800E27EC;
extern int D_800F3428;
extern unsigned short D_800E11F4;
extern unsigned short D_800E11E4;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern unsigned short D_800942EC;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern unsigned char func_800D5010[];
extern int func_80071A54();
extern int func_800CE560();
extern int func_800CE610();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CEDA8();
extern void func_800CEE20();
extern void func_800D004C();
extern void func_800D0728();

int func_800D5290(int mode, unsigned char *a1)
{
    /* retail frame: vector A at sp+0x30, vector B at sp+0x38, buffer at sp+0x40 */
    struct { GteShortVector A; GteShortVector B; B4 buf; } L;
    unsigned char *p;
    int i;
    int r;
    int s0;
    int s1;
    int s3;
    int t;
    int w;
    int clut;
    MATRIX *m;

    L.buf = D_800C22BC;
    if (mode == 1) {
        if (D_800E27EC < 6) {
            for (i = 0; i < 2; i++) {
                p = (unsigned char *)func_800CE610(D_800F33E0[2]);
                if (p) {
                    H(p, 0) = H(a1, 0);
                    H(p, 2) = H(a1, 2);
                    H(p, 4) = H(a1, 4);
                    r = func_80071A54();
                    s1 = (r & 0xF) + 0x2B;
                    t = func_80077CF4(W(a1, 8));
                    SH(p, 6) = t * s1 / 4096;
                    t = func_80077DC4(W(a1, 8));
                    SH(p, 0xA) = t * s1 / 4096;
                    r = func_80071A54();
                    SH(p, 8) = -(r & 7) - 0xC;
                    r = func_80071A54();
                    SH(p, 0xC) = (r & 3) + 0x11;
                    r = func_80071A54();
                    SH(p, 0xE) = r;
                    W(a1, 8) = W(a1, 8) + 0x955;
                }
            }
        }
        if (D_800E27EC < 0x18)
            return 0;
        return 1;
    }
    if (mode < 2) {
        if (mode != 0)
            return 0;
        W(a1, 8) = func_80071A54();
        H(a1, 0) = H((unsigned char *)D_800F32D0[2], 0x268);
        H(a1, 2) = H((unsigned char *)D_800F32D0[2], 0x26A);
        H(a1, 4) = H((unsigned char *)D_800F32D0[2], 0x26C);
        return func_800CE560(D_800F33E0[2], 0x10, 0xC, func_800D5010);
    }
    if (mode != 2)
        return 0;
    m = D_800BCFA4[0];
    gte_SetRotMatrix(m);
    gte_SetTransMatrix(m);
    D_800F3368.a68 = 0x20;
    D_800F3368.a6A = 2;
    D_800F3368.a76 = 0x20;
    D_800F3368.a78 = 0x20;
    D_800F3368.a6C = 0;
    D_800F3368.a70 = D_800E2850[D_800E11F4];
    func_800CEDA8(0);
    D_800F3368.a6E = 1;
    D_800F3368.a72 = 0;
    D_800F3368.a74 = 0x64;
    SH(a1, 2) = H(a1, 2) - 6;
    t = func_80077DC4((D_800E27EC << 10) / 24);
    s1 = t / 32;
    L.A.x = 0;
    L.A.y = 0;
    L.A.w = 0;
    L.A.z = D_800E27EC << 4;
    t = func_80077DC4((D_800E27EC << 10) / 12);
    s0 = 0x3000 - t;
    s3 = (D_800E27EC << 3) / 12;
    if (s3 >= 8)
        s3 = 7;
    w = D_800E1204[D_800F3368.a6C];
    if (D_800F3368.a6C == 4 && D_800F3428 != 0)
        w += 5;
    else
        w += 1;
    clut = func_80077AA4(0, w);
    func_800CEE20(a1, &L.A, s0, s0, D_800F3368.a6A * s3, clut & 0xFFFF, 1, s1, 0);
    if (D_800E27EC < 9) {
        t = func_80077DC4(D_800E27EC << 7);
        s1 = t / 32;
        func_800D004C(a1, 0xFA, 0xFA, 0xC, 0, s0, s0, &L.buf, 0, s1, 1);
    }
    D_800F3368.a74 = 4;
    if ((unsigned)(D_800E27EC - 4) < 0xD) {
        L.B.x = H(a1, 0);
        L.B.y = H(a1, 2);
        L.A.x = 0x400;
        L.A.z = D_800E27EC << 7;
        L.A.y = 0;
        L.A.w = 1;
        L.B.z = H(a1, 4);
        s1 = 0x80 - (((D_800E27EC - 4) << 7) / 12);
        t = func_80077CF4(((D_800E27EC - 4) << 10) / 12);
        s0 = t << 1;
        L.B.y = D_800942EC;
        func_800D0728(&L.B, 0xFA, 0x190, 0xC, &L.A, s0, s0, 0, &L.buf, s1, 1);
    }
    D_800F3368.a68 = 0x20;
    D_800F3368.a6A = 2;
    D_800F3368.a76 = 0x20;
    D_800F3368.a78 = 0x20;
    D_800F3368.a6C = 0;
    D_800F3368.a70 = D_800E2850[D_800E11E4];
    func_800CEDA8(0);
    D_800F3368.a6E = 0;
    D_800F3368.a72 = 0;
    D_800F3368.a74 = 0x40;
    return 0;
}
