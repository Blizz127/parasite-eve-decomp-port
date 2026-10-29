/* room_m0269i — func_8019070C, blob offset 0x1724, 0x62C bytes. Profile era_o2_g0.
 * Particle callback registered by func_80190D38. cmd 1: state 0 drifts (+0x10, +8)
 * for 0x18 frames; state 1 flies by its velocity with 127/128 drag, reflecting
 * vy at D_800942EC, for 0x10 frames; state 2 grows +0x80 for 8 frames.
 * cmd 2: state 0 blends between D_80192334 and an offset point (func_800783E4)
 * and draws func_800D2B58; state 1 draws a func_800CEE20 glow; state 2 draws a
 * func_800D2370 sprite.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros; count
 * separately from plain C.
 * Levers: `goto end; return 1;` tails; state 0 increments +0x12 first; one int `s2`
 * shared by the cmd-2 state-1 glow size and state-2 `a1 + 8` (homes both in $s0),
 * with the state-1 cos value computed in a single-set temp `c` and copied into
 * `s2` just before the call (keeps the 1204*n chain ahead of the cos scaling);
 * `t = (t + 0x1000) / 2` reused for the state-0 scale. */
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

extern B4 D_8018F048;
extern GteShortVector D_80192334;
extern unsigned char D_80192280[];
extern short D_800942EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CFB7C(void *, short, void *);
extern void func_800783E4();
extern void func_800CF3AC();
extern void func_800D2B58();
extern void func_800CEE20();
extern void func_800D2370();

int func_8019070C(int mode, unsigned char *a1)
{
    GteShortVector v;
    GteShortVector d;
    B4 buf;
    int t;
    int s2;
    int s0;
    int w;
    int c;
    unsigned char *q;

    buf = D_8018F048;
    switch (mode) {
    case 1:
        switch (SH(a1, 0x10)) {
        case 0:
            H(a1, 0x12)++;
            H(a1, 8) += 0x10;
            H(a1, 0xA) += 8;
            if (SH(a1, 0x12) < 0x18)
                goto end;
            return 1;
        case 1:
            H(a1, 0x12)++;
            H(a1, 0) += H(a1, 8);
            H(a1, 2) += H(a1, 0xA);
            H(a1, 4) += H(a1, 0xC);
            SH(a1, 8) = SH(a1, 8) * 127 / 128;
            SH(a1, 0xC) = SH(a1, 0xC) * 127 / 128;
            {
                short y = SH(a1, 0xA) + 1;
                SH(a1, 0xA) = y;
                if (SH(a1, 2) >= D_800942EC) {
                    int n = y;
                    SH(a1, 0xA) = -n;
                }
            }
            if (SH(a1, 0x12) < 0x10)
                goto end;
            return 1;
        case 2:
            H(a1, 0x12)++;
            H(a1, 0xC) += 0x80;
            if (SH(a1, 0x12) < 8)
                goto end;
            return 1;
        }
        return 0;
    case 2:
        switch (SH(a1, 0x10)) {
        case 0:
            t = func_80077CF4((SH(a1, 0x12) << 10) / 24);
            t = (t + 0x1000) / 2;
            func_800CFB7C(a1 + 8, SH(a1, 6) * t / 4096, &v);
            v.x += D_80192334.x;
            v.y += D_80192334.y;
            v.z += D_80192334.z;
            t = func_80077CF4((SH(a1, 0x12) << 10) / 24);
            func_800783E4(&D_80192334, &v, 0x1000 - t, t, &d);
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            func_800CF3AC(D_80192280, &buf, SH(a1, 0x12));
            func_800D2B58(&d, &v, 0, &buf, 0, 0x80, 1);
            return 0;
        case 1:
            c = func_80077CF4(SH(a1, 0x12) << 6) / 2 + 0x1000;
            s0 = func_80077DC4(SH(a1, 0x12) * 1204 / 16) / 128 + 0x28;
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0)
                w += 4;
            s2 = c;
            func_800CEE20(a1, 0, s2, s2, D_800F3368.a6A * (SH(a1, 0x12) / 2),
                          func_80077AA4(0, w) & 0xFFFF, 1, s0, 0);
            return 0;
        case 2:
            s0 = func_80077DC4(SH(a1, 0x12) << 7) / 32;
            t = func_80077CF4(SH(a1, 0x12) << 7);
            s2 = (int)(a1 + 8);
            func_800CFB7C((void *)s2, SH(a1, 6) * t / 4096, &v);
            v.x += D_80192334.x;
            v.y += D_80192334.y;
            v.z += D_80192334.z;
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            *(int *)&buf = 0x808080;
            D_800F3368.a70 = D_800E2850[D_800E11EA];
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 0;
            func_800D2370(&v, s2, 0x258, 0x64, 0x70, 0xA0, 0x40, 0x20,
                          func_80077AA4(0x90, D_800E120A) & 0xFFFF, &buf, &buf, (short)s0, 1);
            return 0;
        }
        return 0;
    }
end:
    return 0;
}
