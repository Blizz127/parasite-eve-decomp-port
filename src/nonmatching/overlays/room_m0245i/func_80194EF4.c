/* NON_MATCHING: functionally equivalent to retail room_m0245i 0x5F0C..0x65FC
 * (func_80194EF4; spimdisasm marks it handwritten for its inline cop2 loads).
 * Written from the retail asm, not byte-matched; native port differential-tested
 * by tools/analysis/e21_diff.py.
 * Triceratops beam battle effect main: mode 0 loads the beam mesh
 * (func_8006E498), sets up the blend and sound and starts a 16-particle
 * func_80194C54 pool; mode 1 grows the beam width d[0] to 0x80 and spawns a glow
 * particle per frame (done at frame 0x30); mode 2 draws the two rings
 * (func_800D004C / func_800D0728) and the scaled, spinning mesh at joint 5. */
typedef struct { int m[8]; } Mtx;
typedef struct { short r[10]; int t[3]; } RMtx;
extern unsigned char D_8018F1D4[8];
extern unsigned char D_8018F1E0[4];
extern unsigned char D_801957BC[];
extern short D_80195B68[], D_80195B6A[], D_80195B6C[];
extern int D_80195B70;
extern unsigned char D_80195B74[];
extern int D_800B0E64;
extern int *D_800F32D0;
extern int *D_800F33E0;
extern int D_800E27EC;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern unsigned short D_800E11EA, D_800E11FA;
extern int D_800F3428;
extern unsigned short D_800F336C;
extern short D_800F336A, D_800F3368, D_800F3376, D_800F3378;
extern short D_800F336E, D_800F3370, D_800F3372, D_800F3374;
extern Mtx *D_800BCFA4;
extern int func_8006E498(int a0, int a1);
extern void func_800C6D5C(int mesh, int a1, int a2);
extern void func_800CF4B4(int a0, int a1, void *a2);
extern int func_800D3FD8(void);
extern void func_800D3F64(int id, int a1);
extern int func_800CE560(int pool, int n, int life, void *fn);
extern short *func_800CE610(int pool);
extern int func_80071A54(void);
extern void func_800CE8F0(int pool, int idx, void *local, short *out);
extern int func_80077CF4(int a);
extern int func_80077DC4(int a);
extern void func_800CF3AC(void *table, void *work, int frame);
extern void func_800D004C(short *pos, int a1, int a2, int a3, int s10, int s14, int s18,
                          void *s1c, int s20, int s24, int s28);
extern void func_800D0728(short *pos, int a1, int a2, int a3, int s10, int s14, int s18,
                          void *s1c, int s20, int s24, int s28);
extern int func_80077A64(int tp, int abr, int x, int y);
extern int func_80077AA4(int a0, int a1);
extern void func_800C6EC0(int tpage, int clut);
extern void func_800C6ED8(int a);
extern void func_80079754(short *ang, RMtx *m);
extern void func_80078CC4(RMtx *m, int *scale);
extern void func_800C6EF8(int mesh);
extern void func_800C7098(int mesh, int r, int g, int b);
extern void func_800C6FA0(int mesh, int a);
extern void func_800C71E4(int mesh, RMtx *m);
extern void func_800C6F4C(int mesh);
extern void func_80194C54();

#define GTE_VIEW(m) do {                                                        \
        m = D_800BCFA4;                                                         \
        asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( m ) : "$12", "$13", "$14") ; \
        asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( m ) : "$12", "$13", "$14") ; \
    } while (0)
#define CLUT(a0, c) (c = D_800E1204[D_800F336C],                      \
                     (D_800F336C == 4 && D_800F3428) ? c += 4 : 0,    \
                     func_80077AA4(a0, c) & 0xFFFF)

int func_80194EF4(int mode, short *d)
{
    struct {
        unsigned char a[8];     /* sp+0x30 */
        short p[4];             /* sp+0x38 */
        short ang[4];           /* sp+0x40 */
        unsigned char w[8];     /* sp+0x48 */
        unsigned char b[8];     /* sp+0x50 */
        RMtx mt;                /* sp+0x58 */
        int sc[3];              /* sp+0x78 */
    } f;
    Mtx *m;
    short *p;
    int i, c, fr, s0, s3, s5, v;

    for (i = 0; i < 8; i++) f.a[i] = D_8018F1D4[i];
    for (i = 0; i < 4; i++) f.b[i] = D_8018F1E0[i];
    if (mode == 0) {
        d[0] = 0;
        D_80195B70 = func_8006E498(D_800B0E64, 0xC54A5704);
        func_800C6D5C(D_80195B70, 0, 0);
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3370 = D_800E2850[D_800E11EA];
        func_800CF4B4(3, -1, D_80195B74);
        func_800D3F64(0x5A4, func_800D3FD8());
        return func_800CE560(D_800F33E0[2], 0x10, 0x10, func_80194C54);
    }
    if (mode == 1) {
        if (d[0] < 0x80)
            d[0] = d[0] + 8;
        if (D_800E27EC >= 2) {
            D_800F336C = 3;
            D_800F336E = 0;
            D_800F3370 = D_800E2850[D_800E11EA];
            func_800CF4B4(3, D_800E27EC * 2, D_80195B74);
        }
        p = func_800CE610(D_800F33E0[2]);
        if (p) {
            p[0] = func_80071A54();
            p[1] = func_80071A54();
            p[2] = func_80071A54();
            p[5] = 0;
            p[4] = 0;
            p[6] = (func_80071A54() & 0x7F) - 0x40;
            p[7] = (func_80071A54() & 0x7F) - 0x40;
        }
        return D_800E27EC < 0x30 ? 0 : 1;
    }
    if (mode != 2)
        return 0;
    D_800F336A = 4;
    D_800F3368 = 0x40;
    D_800F3376 = 0x40;
    D_800F3378 = 0x40;
    D_800F336E = 1;
    D_800F336C = 3;
    D_800F3372 = 0;
    D_800F3374 = 0x18;
    D_800F3370 = D_800E2850[D_800E11FA];
    func_800CE8F0(D_800F32D0[2], 5, f.a, f.p);
    fr = D_800E27EC;
    f.ang[0] = func_80077CF4(fr * 80) / 4;
    f.ang[1] = func_80077DC4(D_800E27EC << 7) / 8;
    fr = D_800E27EC;
    f.ang[3] = 0;
    f.ang[2] = (-fr) << 3;
    s3 = d[0];
    if (fr != 0)
        s3 = (s3 * 2) / 3;
    v = func_80077CF4((fr << 10) / 48);
    s5 = v / 4 + 0x400;
    func_800CF3AC(D_801957BC, f.w, D_800E27EC * 3);
    func_800D004C(f.p, 0x124, 0x124, 0xC, 0, 0x1000, 0x1000, f.b, 0, s3, 1);
    func_800D0728(f.p, 0xC8, 0x1F4, 0xC, 0, 0x1000, 0x1000, f.b, 0, s3 / 2, 1);
    D_80195B68[0] = f.p[0];
    D_80195B6A[0] = f.p[1];
    D_80195B6C[0] = f.p[2];
    GTE_VIEW(m);
    func_800CF3AC(D_801957BC, f.w, D_800E27EC);
    D_800F336C = 3;
    D_800F336E = 0;
    D_800F3370 = D_800E2850[D_800E11EA];
    i = func_80077A64(0, 1, 0, 0);
    s0 = (D_800E2850[D_800E11EA] | i) & 0xFFFF;
    i = CLUT(0x30, c);
    func_800C6EC0(s0, i);
    func_800C6ED8(1);
    func_80079754(f.ang, &f.mt);
    f.mt.t[0] = f.p[0];
    f.mt.t[1] = f.p[1];
    f.mt.t[2] = f.p[2];
    f.sc[0] = s5;
    f.sc[1] = s5;
    f.sc[2] = s5;
    func_80078CC4(&f.mt, f.sc);
    func_800C6EF8(D_80195B70);
    func_800C7098(D_80195B70, f.w[0], f.w[1], f.w[2]);
    func_800C6FA0(D_80195B70, (s3 / 2) & 0xFFFF);
    func_800C71E4(D_80195B70, &f.mt);
    func_800C6F4C(D_80195B70);
    D_800F3368 = 0x20;
    D_800F336A = 2;
    D_800F3376 = 0x20;
    D_800F3378 = 0x20;
    D_800F336C = 3;
    D_800F336E = 0;
    D_800F3374 = 8;
    D_800F3370 = D_800E2850[D_800E11EA];
    return 0;
}
