/* NON_MATCHING: functionally equivalent to retail room_m0089i 0x4630..0x5008
 * (func_80193618).  Written from the retail asm, not byte-matched; native port
 * differential-tested by tools/analysis/e21_diff.py.
 * Battle effect callback (Day 2 Central Park random battles, m0089i), the
 * second effect after func_80192CD4: mode 0 records the origin, creates the
 * two effect models and the particle list (func_800CE560 with the overlay's
 * particle callback 0x80193200); mode 1 spawns particles on the frame counter
 * D_800E27EC, ends at frame 64, and while the effect touches the target
 * before frame 40 flags the hit; mode 2 draws the three spinning rings, the
 * flash model and the shadow sprite. */
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern int D_8019416C;
extern int D_80194168;
extern int D_800B0E64;
extern int D_800E27EC;
extern int D_800F3428;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern unsigned char *D_8009D254;
extern unsigned short D_800E11FA;
extern unsigned short D_800E11EA;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern short D_800F3368, D_800F336A, D_800F336C, D_800F336E, D_800F3370;
extern short D_800F3372, D_800F3374, D_800F3376, D_800F3378;
extern int func_8006E498();
extern void func_800C6D5C();
extern int func_800CE560();
extern short *func_800CE610();
extern int func_80071A54();
extern int func_800C6B90();
extern int func_80077DC4();
extern int func_80077CF4();
extern int func_80077AA4();
extern int func_80077A64();
extern void func_800C6EC0();
extern void func_800C6ED8();
extern void func_80079754();
extern void func_80078CC4();
extern void func_800C6EF8();
extern void func_800C6FA0();
extern void func_800C71E4();
extern void func_800C6F4C();
extern void func_800CF3AC();
extern void func_800D004C();

#define W(p, o) (*(int *)((unsigned char *)(p) + (o)))
#define P(p, o) (*(unsigned char **)((unsigned char *)(p) + (o)))

#define TEX_PAGE(add_hi, add_lo, a64)                                                     \
    do {                                                                                  \
        D_800F336C = 3;                                                                   \
        D_800F336E = 1;                                                                   \
        D_800F3370 = D_800E2850[D_800E11FA];                                              \
        k = (D_800E2850[D_800E11FA] | func_80077A64(0, (a64), 0, 0)) & 0xFFFF;            \
        c = D_800E1204[(unsigned short)D_800F336C];                                       \
        if ((unsigned short)D_800F336C == 4 && D_800F3428 != 0)                           \
            c += (add_hi);                                                                \
        else                                                                              \
            c += (add_lo);                                                                \
        func_800C6EC0(k, func_80077AA4(0, c) & 0xFFFF);                                   \
        func_800C6ED8(1);                                                                 \
    } while (0)

#define DRAW_MODEL(mdl, frame, m)  \
    do {                           \
        func_800C6EF8(mdl);        \
        func_800C6FA0(mdl, frame); \
        func_800C71E4(mdl, m);     \
        func_800C6F4C(mdl);        \
    } while (0)

int func_80193618(int mode, short *q, short *a2)
{
    /* retail frame order: sp+48 .. sp+240 */
    struct {
        SVECTOR ang;
        unsigned char o56[8];
        MATRIX m1;
        VECTOR s1;
        MATRIX m2;
        VECTOR s2;
        MATRIX m3;
        VECTOR s3;
        MATRIX m4;
    } f;
    unsigned char *p;
    unsigned char *x;
    short *e;
    int i;
    int t;
    int fr1;
    int fr2;
    int s;
    int s19;
    int s22;
    int s17;
    int s20;
    int k;
    int c;

    if (mode == 1) {
        if (D_800E27EC == 1) {
            for (i = 0; i < 16; i++) {
                e = func_800CE610(W(D_800F33E0, 8));
                if (e != 0) {
                    e[4] = 0;
                    e[5] = (func_80071A54() & 0x7F) + (i << 8);
                    e[6] = (func_80071A54() & 0xFF) + 500;
                }
            }
        }
        t = D_800E27EC;
        if (t < 56 && (t & 1)) {
            e = func_800CE610(W(D_800F33E0, 8));
            if (e != 0) {
                e[4] = 1;
                e[0] = (func_80071A54() & 0x7F) + 512;
                e[1] = func_80071A54();
                e[2] = 0;
            }
        }
        if (D_800E27EC >= 64)
            return 1;
        if (func_800C6B90(q, 240) == 0)
            return 0;
        if (D_800E27EC >= 40)
            return 0;
        if (D_800E2368[13] == 0)
            return 0;
        x = D_800F32D0;
        if ((W(P(P(x, 8), 0), 0) & 0x3F000000) != 0x01000000)
            return 0;
        p = P(D_8009D254, 0);
        W(p, 76) |= 0x4000;
        p = P(P(x, 8), 0);
        W(p, 0) = (W(p, 0) & 0xC0FFFFFF) | 0x29000000;
        p = P(P(x, 8), 0);
        W(p, 0) |= 0x80000000;
        return 0;
    }
    if (mode == 0) {
        q[4] = 0;
        q[5] = 0;
        q[0] = a2[0];
        q[1] = a2[1];
        q[2] = a2[2];
        if (D_800E2368[13] != 0) {
            p = P(D_800F32D0, 8);
            if (p != 0) {
                p = P(p, 0);
                if (p != 0) {
                    p = P(p, 24);
                    if (p[0] == 1)
                        p[0] = 2;
                }
            }
        }
        D_8019416C = func_8006E498(D_800B0E64, 0xC5464704);
        func_800C6D5C(D_8019416C, 0, 0);
        D_80194168 = func_8006E498(D_800B0E64, 0xC5864704);
        func_800C6D5C(D_80194168, 0, 0);
        return func_800CE560(W(D_800F33E0, 8), 16, 24, 0x80193200);
    }
    if (mode != 2)
        return 0;
    fr1 = func_80077DC4(D_800E27EC << 5) / 32;
    s = func_80077DC4(D_800E27EC << 5);
    s19 = s + 1024;
    s22 = func_80077CF4(D_800E27EC << 5) + 1024;
    fr2 = func_80077DC4(D_800E27EC << 5) / 32;
    s17 = func_80077CF4(D_800E27EC << 5) + 2048;
    s20 = 2048 - func_80077CF4(D_800E27EC << 5) / 4;
    D_800F3374 = 16;
    s19 = s19 / 4;
    s17 = s17 / 3;
    if (D_800E27EC < 33) {
        f.ang.vx = 0;
        f.ang.vy = -(D_800E27EC << 5);
        f.ang.vz = 0;
        TEX_PAGE(6, 2, 1);
        func_80079754(&f.ang, &f.m1);
        f.m1.t[0] = q[0];
        f.m1.t[1] = q[1];
        f.m1.t[2] = q[2];
        f.s1.vx = s19;
        f.s1.vy = s22;
        f.s1.vz = s19;
        func_80078CC4(&f.m1, &f.s1);
        DRAW_MODEL(D_8019416C, fr1 & 0xFFFF, &f.m1);
        f.ang.vx = 0;
        f.ang.vz = 0;
        f.ang.vy = D_800E27EC << 5;
        func_80079754(&f.ang, &f.m2);
        f.m2.t[0] = q[0];
        f.m2.t[1] = q[1];
        f.m2.t[2] = q[2];
        f.s2.vx = s17;
        f.s2.vy = s20;
        f.s2.vz = s17;
        func_80078CC4(&f.m2, &f.s2);
        DRAW_MODEL(D_8019416C, fr2 & 0xFFFF, &f.m2);
        f.ang.vx = 0;
        f.ang.vz = 0;
        f.ang.vy = D_800E27EC << 4;
        func_80079754(&f.ang, &f.m3);
        f.m3.t[0] = q[0];
        f.m3.t[1] = q[1];
        f.s3.vx = s19;
        f.s3.vy = (s20 * 3) / 2;
        f.s3.vz = s19;
        f.m3.t[2] = q[2];
        func_80078CC4(&f.m3, &f.s3);
        DRAW_MODEL(D_8019416C, fr2 & 0xFFFF, &f.m3);
    }
    if (D_800E27EC < 25) {
        t = (D_800E27EC << 10) / 24;
        f.ang.vx = 0;
        f.ang.vy = D_800E27EC << 6;
        f.ang.vz = 0;
        fr1 = func_80077DC4(t) / 32;
        s = (func_80077CF4((D_800E27EC << 10) / 24) * 3) / 2;
        TEX_PAGE(7, 3, 3);
        func_80079754(&f.ang, &f.m4);
        f.m4.t[0] = q[0];
        f.m4.t[1] = q[1];
        f.s3.vx = s;
        f.s3.vy = s;
        f.s3.vz = s;
        f.m4.t[2] = q[2];
        func_80078CC4(&f.m4, &f.s3);
        DRAW_MODEL(D_80194168, fr1 & 0xFFFF, &f.m4);
    }
    if (D_800E27EC < 49) {
        func_800CF3AC(0x801940F8, f.o56, D_800E27EC);
        func_800D004C(q, 500, 500, 12, 0, 4096, 4096, f.o56, 0, 128, 1);
    }
    D_800F3368 = 32;
    D_800F336A = 2;
    D_800F3376 = 32;
    D_800F3378 = 32;
    D_800F336C = 3;
    D_800F336E = 0;
    D_800F3372 = 0;
    D_800F3370 = D_800E2850[D_800E11EA];
    return 0;
}
