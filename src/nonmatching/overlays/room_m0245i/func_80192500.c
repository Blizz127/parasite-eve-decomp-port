/* NON_MATCHING: functionally equivalent to retail room_m0245i 0x3518..0x425C
 * (func_80192500; spimdisasm marks it handwritten for its inline cop2 loads).
 * Written from the retail asm, not byte-matched; native port differential-tested
 * by tools/analysis/e21_diff.py.
 * Triceratops fire breath particle callback.  q[0..2] position, q[3] spine
 * index into D_8019581C (24-point spines, 200 bytes each), q[4..6] heading,
 * q[7] dying counter, q[8] speed, q[9] turn, q[10] kind (0 spawn, 1 the breath
 * head that walks its spine and burns the player, 2 ember, 3 puff), q[11] age.
 * Mode 1 steps it, mode 2 draws it (kind 1 as a func_800D3114 ribbon). */
extern unsigned char D_8018F1CC[4], D_8018F1D0[4], D_8018F1D4[8];
extern short D_8019581C[];
extern int **D_8009D254;
extern int *D_800F32D0;
extern int *D_800F33E0;
extern unsigned char *D_800E2368;
extern int D_800E27EC;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern unsigned short D_800E11EA, D_800E120A;
extern int D_800F3428;
extern unsigned short D_800F336C;
extern short D_800F336A, D_800F3368, D_800F3376, D_800F3378;
extern short D_800F336E, D_800F3370;
extern void *D_800BCFA4;
extern void func_800CE8F0(int pool, int idx, void *local, short *out);
extern void func_800CE9D4(int pool, int a1, short *dir);
extern void func_800CE870(int who, int a1, short *out);
extern void func_800CFAA8(short *a, short *b, short *out);
extern void func_800CFD50(short *a, short *b, int c);
extern void func_800CFB7C(short *dir, int dist, short *out);
extern void func_800D3AFC(short *pts, int n, short *q, int a3);
extern short *func_800CE610(int pool);
extern int func_80071A54(void);
extern int func_800C6B90(short *pos, int r);
extern int func_80077DC4(int a);
extern int func_80077AA4(int a0, int a1);
extern void func_800D3114(short *pts, int n, int width, int u0, int v0, int du, int dv,
                          int tpage, int clut, int scale, void *col0, void *col1, int abr);
extern void func_800D004C(short *pos, int a1, int a2, int a3, int s10, int s14, int s18,
                          void *s1c, int s20, int s24, int s28);
extern void func_800D2370(void *pos, short *q, int a2, int a3, int s10, int s14, int s18,
                          int s1c, int clut, void *c0, void *c1, int scale, int abr);
extern void func_800CEE20(short *pos, void *ang, int sx, int sy, int tex, int clut,
                          int abr, int bright, void *extra);

#define GTE_VIEW(m) do {                                                        \
        m = D_800BCFA4;                                                         \
        asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( m ) : "$12", "$13", "$14") ; \
        asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( m ) : "$12", "$13", "$14") ; \
    } while (0)
#define CLUT(a0, c) (c = D_800E1204[D_800F336C],                      \
                     (D_800F336C == 4 && D_800F3428) ? c += 4 : 0,    \
                     func_80077AA4(a0, c) & 0xFFFF)

#define SPINE(q) (&D_8019581C[(q)[3] * 100])

int func_80192500(int mode, short *q)
{
    struct {
        unsigned char c0[8];    /* sp+0x38 */
        unsigned char c1[8];    /* sp+0x40 */
        unsigned char a[8];     /* sp+0x48 */
        short e[4];             /* sp+0x50 */
        short d[4];             /* sp+0x58 */
    } f;
    short *p, *s;
    void *m;
    int i, c, s0, s1, v, kind;

    for (i = 0; i < 4; i++) f.c0[i] = D_8018F1CC[i];
    for (i = 0; i < 4; i++) f.c1[i] = D_8018F1D0[i];
    for (i = 0; i < 8; i++) f.a[i] = D_8018F1D4[i];
    kind = q[10];
    if (mode == 1) {
        if (kind == 0) {
            q[11] = 0;
            q[10] = 1;
            q[7] = 0;
            func_800CE8F0(D_800F32D0[2], 7, f.a, q);
            func_800CE9D4(D_800F32D0[2], 0, f.d);
            f.d[0] = 0;
            q[4] = 0;
            q[5] = f.d[1];
            q[6] = f.d[2];
            func_800D3AFC(SPINE(q), 0x18, q, 1);
            q[8] = 0x20;
            q[9] = 0x200;
            p = func_800CE610(D_800F33E0[2]);
            if (p) {
                p[0] = q[0];
                p[1] = q[1];
                p[10] = 3;
                p[11] = 0;
                p[2] = q[2];
            }
            return 0;
        }
        if (kind == 1) {
            q[11] = q[11] + 1;
            s = SPINE(q);
            func_800CE870((int)D_8009D254, 1, f.e);
            func_800CFAA8(q, f.e, f.d);
            f.d[0] = 0;
            if (q[7] == 0) {
                func_800CFD50(f.d, &q[4], (unsigned short)q[9]);
                if (q[11] >= 3) {
                    s0 = (q[11] << 4) + 0x100;
                    if ((func_80071A54() & 1) == 0)
                        v = func_80071A54() % (s0 * 2) - s0;
                    else
                        v = func_80071A54() % s0 - (s0 >> 1);
                    q[5] = q[5] + v;
                }
                if (q[8] < 0x3C)
                    q[8] = q[8] + 2;
                func_800CFB7C(&q[4], (short)((q[8] * q[8]) / 16), f.e);
                q[0] = q[0] + f.e[0];
                q[1] = q[1] + f.e[1];
                q[2] = q[2] + f.e[2];
            } else {
                q[7] = q[7] + 1;
                if (q[7] >= 0x10)
                    return 1;
            }
            func_800D3AFC(s, 0x18, q, 0);
            s = SPINE(q);
            for (i = 0; i < 0x18; i++, s += 4) {
                int *x;
                if (!func_800C6B90(s, 0x68) || !D_800E2368[0xD])
                    continue;
                if ((**(int **)D_800F32D0[2] & 0x3F000000) != 0x1000000)
                    continue;
                (*D_8009D254)[0x13] |= 0x4000;
                x = *(int **)D_800F32D0[2];
                *x |= 0x3F000000;
                x = *(int **)D_800F32D0[2];
                *x |= 0x80000000;
            }
            s = SPINE(q);
            if (q[7] == 0 && (p = func_800CE610(D_800F33E0[2])) != 0) {
                s += (func_80071A54() & 3) * 16;
                p[0] = s[0];
                p[1] = s[1];
                p[2] = s[2];
                p[0] = p[0] - 0x20 + (func_80071A54() & 0x3F);
                p[1] = p[1] - 0x20 + (func_80071A54() & 0x3F);
                p[2] = p[2] - 0x20 + (func_80071A54() & 0x3F);
                p[4] = q[4];
                p[5] = q[5];
                p[6] = q[6];
                p[4] = p[4] - 100 + func_80071A54() % 200;
                p[5] = p[5] - 100 + func_80071A54() % 200;
                p[8] = (func_80071A54() & 0x3F) - 0x20;
                p[9] = (func_80071A54() & 0x3F) - 0x20;
                p[10] = 2;
                p[11] = 0;
            }
            if (D_800E27EC < 0x20 || q[7] != 0)
                return 0;
            p = func_800CE610(D_800F33E0[2]);
            if (p) {
                p[0] = q[0];
                p[1] = q[1];
                p[10] = 3;
                p[11] = 0;
                p[2] = q[2];
            }
            q[7] = 1;
            return 0;
        }
        if (kind == 2) {
            q[11] = q[11] + 1;
            q[4] = q[4] + q[9];
            q[5] = q[5] + q[8];
            return q[11] < 0x14 ? 0 : 1;
        }
        if (kind == 3) {
            q[11] = q[11] + 1;
            return q[11] < 0x18 ? 0 : 1;
        }
        return 0;
    }
    if (mode != 2)
        return 0;
    if (kind == 1) {
        s1 = ((D_800E27EC / 3) & 1) * 32 + 0x20;
        GTE_VIEW(m);
        i = CLUT(0x10, c);
        func_800D3114(SPINE(q), 0x18, 0x78, 0, s1, 0xFD, 0x1F, D_800E2850[D_800E11EA], i, 0x80,
                      f.c0, f.c1, 1);
        i = func_80071A54();
        func_800D004C(SPINE(q) + (i & 0xF) * 4, 0x190, 0x190, 6, 0, 0x1000, 0x1000, f.c0, 0,
                      0x80, 3);
        return 0;
    }
    if (kind == 2) {
        s1 = func_80077DC4((q[11] << 10) / 20) / 32;
        func_80077DC4((q[11] << 10) / 20);
        s0 = (D_800E27EC & 1) << 7;
        GTE_VIEW(m);
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3370 = D_800E2850[D_800E11EA];
        i = func_80077AA4(0x10, D_800E120A) & 0xFFFF;
        func_800D2370(q, &q[4], 0x1FE, 0x78, s0, 0, 0x80, 0x20, i, f.c1, f.c0, (short)s1, 1);
        if (q[11] >= 0xD)
            return 0;
        s1 = func_80077DC4((q[11] << 10) / 12) / 64;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        i = CLUT(0x40, c);
        func_800CEE20(q, 0, 0x1000, 0x1000, D_800F336A * ((q[11] * 2) / 3) + 0x90, i, 1, s1, 0);
        return 0;
    }
    if (kind == 3) {
        s1 = func_80077DC4((q[11] << 10) / 24) / 32;
        func_80077DC4((q[11] << 10) / 24);
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        f.e[0] = q[0];
        f.e[1] = q[1];
        f.e[2] = q[2];
        f.e[0] = f.e[0] - 0x10 + (func_80071A54() & 0x1F);
        f.e[1] = f.e[1] - 0x10 + (func_80071A54() & 0x1F);
        f.e[2] = f.e[2] - 0x10 + (func_80071A54() & 0x1F);
        i = CLUT(0x40, c);
        func_800CEE20(f.e, 0, 0x1000, 0x1000, D_800F336A * (q[11] & 7) + 0x90, i, 1, s1, 0);
        return 0;
    }
    return 0;
}
