/* func_800D5A00 -- EXE, VRAM 0x800D5A00, file offset 0xC6200, 0x2E4 bytes.
 * NON_MATCHING behaviour C (proven by the e21 differential; not a byte match).
 * Three-mode effect callback (a0 = mode, a1 = effect record):
 *   mode 0: seed the record (rand into +8, +0xC = 0, +0/+2/+4 = halfwords
 *           0x268/0x26A/0x26C of the object at D_800F32D0->+8), then
 *           func_800CE560(D_800F33E0->+8, 8, 0x18, func_800D5898); its result
 *           is the return value.
 *   mode 1: when D_800E27EC is 4..10 spawn up to three particles from
 *           func_800CE610(D_800F33E0->+8); return D_800E27EC >= 0x20.
 *   mode 2: D_800F3374 = 0x40; when D_800E27EC < 13 draw with
 *           f = sin((D_800E27EC << 10) / 12); set the camera matrix
 *           *D_800BCFA4 and func_800CF5B0(a1, 0). Returns 0.
 * Retail copies the 4 bytes at D_800C22C4 to a stack buffer (stack address s2)
 * and passes that buffer to func_800CF3AC / func_800D0728.
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

extern B4 D_800C22C4;
extern int *D_800F32D0;
extern int *D_800F33E0;
extern int D_800E27EC;
extern short D_800F3374;
extern unsigned char D_800E166C[];
extern MATRIX *D_800BCFA4[];

extern unsigned char func_800D5898[];
extern int func_80071A54();
extern int func_800CE560();
extern int func_800CE610();
extern int func_80077CF4();
extern void func_800CF3AC();
extern void func_800D0728();
extern void func_800CF5B0();

int func_800D5A00(int mode, unsigned char *a1)
{
    /* retail keeps the vector at sp+0x30 and the 4-byte buffer at sp+0x38,
     * adjacent; func_800D0728 reads both through its first argument. */
    struct { GteShortVector v; B4 buf; } L;
    unsigned char *p;
    int i;
    int r;
    int s0;
    int k;
    MATRIX *m;

    L.buf = D_800C22C4;
    if (mode == 1) {
        if ((unsigned)(D_800E27EC - 4) < 7) {
            for (i = 0; i < 3; i++) {
                p = (unsigned char *)func_800CE610(D_800F33E0[2]);
                if (p) {
                    SH(p, 2) = W(a1, 0xC);
                    r = func_80071A54();
                    SH(p, 6) = (r & 3) + 0xC;
                    SH(p, 4) = W(a1, 8);
                    r = func_80071A54();
                    W(a1, 8) = W(a1, 8) + 0x8AA + (r & 0x1F);
                }
            }
        }
        if (D_800E27EC < 0x20)
            return 0;
        return 1;
    }
    if (mode < 2) {
        if (mode != 0)
            return 0;
        W(a1, 8) = func_80071A54();
        W(a1, 0xC) = 0;
        H(a1, 0) = H((unsigned char *)D_800F32D0[2], 0x268);
        H(a1, 2) = H((unsigned char *)D_800F32D0[2], 0x26A);
        H(a1, 4) = H((unsigned char *)D_800F32D0[2], 0x26C);
        return func_800CE560(D_800F33E0[2], 8, 0x18, func_800D5898);
    }
    if (mode != 2)
        return 0;
    D_800F3374 = 0x40;
    if (D_800E27EC < 0xD) {
        L.v.x = H(a1, 0);
        L.v.y = H(a1, 2);
        L.v.z = H(a1, 4);
        s0 = func_80077CF4((D_800E27EC << 10) / 12);
        W(a1, 0xC) = s0 / 8;
        func_800CF3AC(D_800E166C, &L.buf, D_800E27EC);
        func_800D0728(&L.v, 0x12C, 0x200, 0xC, 0, s0, s0, 0, &L.buf, 0x80, 1);
    }
    D_800F3374 = 0x40;
    m = D_800BCFA4[0];
    gte_SetRotMatrix(m);
    gte_SetTransMatrix(m);
    func_800CF5B0(a1, 0);
    return 0;
}
