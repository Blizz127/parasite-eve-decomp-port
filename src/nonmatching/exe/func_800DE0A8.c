/* func_800DE0A8 -- EXE, VRAM 0x800DE0A8, file offset 0xCE8A8, 0x700 bytes.
 * NON_MATCHING behaviour C (proven by the e21 differential; not a byte match).
 * Three-mode effect callback (a0 = mode, a1 = effect record):
 *   mode 0: func_800CE870(D_8009D254, 1, record + 8); record +0/+2/+4 = words
 *           0x274/0x278/0x27C of the object at D_8009D254->+0x238;
 *           func_800C6D5C(D_800F3474, 0, 0); returns 0.
 *   mode 1: returns D_800E27EC >= 0x19.
 *   mode 2: f = D_800E27EC. f == 0: func_800D1AE0 with 0xA0A0A0; 1 <= f <= 5:
 *           func_800D1AE0 with 0xA0FFFF at y = 0x80 - ((f - 1) << 5). f < 0x11:
 *           draw the object's matrix scaled by {sin/3, sin/3, cos} through the
 *           func_800C70xx/71E4 draw path (two passes). f < 0x19: colour curve
 *           D_800E20CC, func_800D004C sprite (f < 0x11: func_800D0728 too) and
 *           func_800CEE20 glow. Returns 0.
 * The colour buffer at sp+0x40 is read back after func_800CF3AC (out=3 bytes);
 * its fourth byte is never written -- retail never reads it.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { unsigned char c[4]; } B4;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

extern unsigned char *D_8009D254;
extern int D_800F3474;
extern int D_800E27EC;
extern unsigned char D_800E20CC[];
extern unsigned short D_800E11E6;
extern unsigned short D_800E11F6;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern unsigned short D_800F3368, D_800F336A, D_800F336C, D_800F336E,
                      D_800F3370, D_800F3372, D_800F3374, D_800F3376,
                      D_800F3378;

extern int func_800CE870();
extern int func_800C6D5C();
extern int func_800D1AE0();
extern int func_80077DC4();
extern int func_80077CF4();
extern int func_80078CC4();
extern int func_800CF3AC();
extern int func_800CEDA8();
extern int func_80077A64();
extern int func_80077AA4();
extern int func_800C6EC0();
extern int func_800C6ED8();
extern int func_800C6EF8();
extern int func_800C7098();
extern int func_800C71E4();
extern int func_800C6F4C();
extern int func_800D004C();
extern int func_800D0728();
extern int func_800CEE20();

#define OBJ() (*(unsigned char **)(D_8009D254 + 0x238))

int func_800DE0A8(int mode, unsigned char *a1)
{
    /* retail frame: vec at sp+0x30, colour buffer at sp+0x40, matrix at
     * sp+0x48 (passed by pointer, callees read across the whole block) */
    struct { int vec[3]; int pad0; B4 buf; int pad1; MATRIX m; } L;
    int f;
    int p;
    int s0;
    int a2;
    int v;

    if (mode == 1)
        return D_800E27EC >= 0x19;
    if (mode == 0) {
        func_800CE870(D_8009D254, 1, a1 + 8);
        SH(a1, 0) = W(OBJ(), 0x274);
        SH(a1, 2) = W(OBJ(), 0x278);
        SH(a1, 4) = W(OBJ(), 0x27C);
        func_800C6D5C(D_800F3474, 0, 0);
        return 0;
    }
    if (mode != 2)
        return 0;

    if (D_800E27EC == 0) {
        *(int *)&L.buf = 0xA0A0A0;
        func_800D1AE0(&L.buf, 0x50, 2, 8);
    }
    p = D_800E27EC - 1;
    if ((unsigned)p < 5) {
        *(int *)&L.buf = 0xA0FFFF;
        func_800D1AE0(&L.buf, 0x80 - (p << 5), 1, 8);
    }

    if (D_800E27EC < 0x11) {
        L.m = *(MATRIX *)OBJ();
        f = func_80077DC4(D_800E27EC << 6);
        L.vec[0] = f / 3;
        L.vec[1] = f / 3;
        L.vec[2] = func_80077CF4(D_800E27EC << 6);
        func_80078CC4(&L.m, L.vec);
        L.m.t[0] = W(OBJ(), 0x274);
        L.m.t[1] = W(OBJ(), 0x278);
        L.m.t[2] = W(OBJ(), 0x27C);
        func_800CF3AC(D_800E20CC, &L.buf, D_800E27EC);
        D_800F336C = 1;
        D_800F3370 = D_800E2850[D_800E11E6];
        func_800CEDA8(1);
        D_800F336E = 0;
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        s0 = (D_800E2850[D_800E11E6] | func_80077A64(0, 1, 0, 0)) & 0xFFFF;
        a2 = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0)
            a2 += 4;
        v = func_80077AA4(0xD0, a2);
        func_800C6EC0(s0, v & 0xFFFF);
        func_800C6ED8(1);
        func_800C6EF8(D_800F3474);
        func_800C7098(D_800F3474, L.buf.c[0], L.buf.c[1], L.buf.c[2]);
        func_800C71E4(D_800F3474, &L.m);
        L.vec[0] = 0x1800;
        L.vec[1] = 0x1800;
        L.vec[2] = 0x1000;
        func_80078CC4(&L.m, L.vec);
        func_800C7098(D_800F3474, L.buf.c[0], L.buf.c[1] >> 1, 0);
        s0 = (D_800E2850[D_800E11E6] | func_80077A64(0, 1, 0, 0)) & 0xFFFF;
        a2 = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0)
            a2 += 4;
        v = func_80077AA4(0xD0, a2);
        func_800C6EC0(s0, v & 0xFFFF);
        func_800C71E4(D_800F3474, &L.m);
        func_800C6F4C(D_800F3474);
    }

    if (D_800E27EC < 0x19) {
        f = D_800E27EC;
        D_800F3374 = 0x10;
        func_800CF3AC(D_800E20CC, &L.buf, (f << 4) / 24);
        func_800D004C(a1, 0x258, 0x258, 0xC, 0, 0x1000, 0x1000, &L.buf, 0, 0x80, 1);
        f = D_800E27EC;
        if (f < 0x11) {
            L.buf.c[2] = 0;
            s0 = func_80077CF4(f << 6) + 0x800;
            func_800CF3AC(D_800E20CC, &L.buf, D_800E27EC);
            func_800D0728(a1, 0x1F4, 0x258, 0x14, 0, s0, s0, &L.buf, 0, 0x40, 1);
            f = D_800E27EC;
        }
        a2 = func_80077DC4((f << 10) / 24);
        *(int *)&L.buf = 0xC8B4B4;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        D_800F336C = 1;
        D_800F3370 = D_800E2850[D_800E11F6];
        func_800CEDA8(1);
        D_800F336E = 1;
        D_800F3372 = 0;
        v = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428 != 0)
            v += 7;
        else
            v += 3;
        v = func_80077AA4(0, v);
        func_800CEE20(a1, 0, 0x2000, 0x2000, 0x42, v & 0xFFFF, 1, a2 / 32, &L.buf);
    }
    return 0;
}
