/* NON_MATCHING: functionally equivalent to retail room_m0188i 0x3534..0x40CC
 * (func_8019251C).  Written from the retail asm, not byte-matched; native port
 * differential-tested by tools/analysis/e21_diff.py.
 * Particle callback of the m0188i battle effect pool.  q[0..2] position,
 * q[4..6] velocity, q[8] kind (0 falling body, 1 floor impact, 2 rising spray,
 * 3 debris), q[9] age, q[10] kind-0 phase.  Mode 1 steps it: kind 0 drops onto
 * the player's position, spawning spray/debris on odd frames, and kinds 0/1
 * flag the player hit (body +0x4C 0x4000, target state 0x19 + 0x80000000) when
 * it lands within range; mode 2 draws it. */
typedef struct {
    short vx, vy, vz, pad;
} SVec;

extern unsigned char D_8018F1CC[8];
extern unsigned char D_8018F1D4[8];
extern int **D_8009D254;
extern int *D_800F32D0;
extern int *D_800F33E0;
extern unsigned char *D_800E2368;
extern int D_800E27EC;
extern short D_800942EC;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned short D_800F336C;
extern short D_800F336A, D_800F3368, D_800F3376, D_800F3378;
extern void func_800CE870(int **who, int a1, SVec *out);
extern int func_80071A54(void);
extern short *func_800CE610(int pool);
extern int func_800C6B90(short *pos, int r);
extern int func_80077DC4(int a);
extern int func_80077CF4(int a);
extern int func_80077AA4(int a0, int a1);
extern void func_800CEE20(short *pos, void *ang, int sx, int sy, int tex, int clut,
                          int abr, int bright, void *extra);

#define CLUT(a0, c) (c = D_800E1204[D_800F336C],                      \
                     (D_800F336C == 4 && D_800F3428) ? c += 4 : 0,    \
                     func_80077AA4(a0, c) & 0xFFFF)

/* kinds 0/1 landing on the player: mark Aya's body hit and the target slot */
#define HIT_PLAYER(x) (x = *(int **)D_800F32D0[2],                                \
                       ((*x & 0x3F000000) == 0x1000000)                           \
                       ? ((*D_8009D254)[0x13] |= 0x4000,                          \
                          *x = (*x & 0xC0FFFFFF) | 0x19000000,                    \
                          *x |= 0x80000000, 0) : 0)

int func_8019251C(int mode, short *q)
{
    union {
        unsigned char b[8];
        short h[4];
    } a, b;            /* sp+0x28, sp+0x30 */
    SVec t;            /* sp+0x38 */
    short pos[4];      /* sp+0x40 */
    SVec ang;          /* sp+0x48 */
    short *p;
    int *x;
    int i, c, r, s0, s2, kind, sub, go;

    for (i = 0; i < 8; i++) a.b[i] = D_8018F1CC[i];
    for (i = 0; i < 8; i++) b.b[i] = D_8018F1D4[i];
    kind = q[8];
    if (mode == 1) {
        if (kind == 0) {
            sub = q[10];
            if (sub == 0) {
                q[9] = q[9] + 1;
                q[1] = q[1] - 0x6E;
                if (q[9] >= 0x10) {
                    q[10] = 1;
                    q[9] = 0;
                    func_800CE870(D_8009D254, 0, &t);
                    r = func_80071A54();
                    q[0] = t.vx + r % 1200 - 600;
                    r = func_80071A54();
                    q[2] = t.vz + r % 1200 - 600;
                }
                go = q[10] < 2;
            } else if (sub == 1) {
                q[9] = q[9] + 1;
                q[1] = q[1] + 0x6E;
                if (q[1] >= D_800942EC) {
                    q[8] = 1;
                    q[9] = 0;
                    q[1] = D_800942EC;
                }
                go = q[10] < 2;
            } else {
                go = sub < 2;
            }
            if (go && (D_800E27EC & 1)) {
                if (func_80071A54() & 1) {
                    p = func_800CE610(D_800F33E0[2]);
                    if (p) {
                        p[0] = q[0];
                        p[1] = q[1];
                        p[8] = 2;
                        p[9] = 0;
                        p[10] = 0;
                        p[2] = q[2];
                    }
                } else {
                    p = func_800CE610(D_800F33E0[2]);
                    if (p) {
                        p[0] = q[0];
                        p[1] = q[1];
                        p[2] = q[2];
                        p[4] = (func_80071A54() & 0x1F) - 0x10;
                        p[6] = (func_80071A54() & 0x1F) - 0x10;
                        p[5] = (func_80071A54() & 3) - 2;
                        p[8] = 3;
                        p[9] = 0;
                    }
                }
            }
            if (func_800C6B90(q, 0x6E) == 0)
                return 0;
            if (q[1] < D_800942EC - 0x202)
                return 0;
            if (D_800E2368[0xD] == 0)
                return 0;
            HIT_PLAYER(x);
            return 0;
        }
        if (kind == 1) {
            q[9] = q[9] + 1;
            if (D_800E27EC & 1) {
                p = func_800CE610(D_800F33E0[2]);
                if (p) {
                    p[0] = q[0];
                    p[1] = q[1];
                    p[2] = q[2];
                    p[0] = (func_80071A54() & 0xFF) - 0x80;
                    p[2] = (func_80071A54() & 0xFF) - 0x80;
                    p[4] = (func_80071A54() & 0x1F) - 0x10;
                    p[6] = (func_80071A54() & 0x1F) - 0x10;
                    p[5] = (func_80071A54() & 3) - 2;
                    p[8] = 3;
                    p[9] = 0;
                }
            }
            if (func_800C6B90(q, 0xC8) && q[9] < 9 && D_800E2368[0xD])
                HIT_PLAYER(x);
            return q[9] < 0x10 ? 0 : 1;
        }
        if (kind == 2) {
            q[9] = q[9] + 1;
            q[1] = q[1] + q[10];
            q[10] = q[10] + 2;
            return q[9] < 8 ? 0 : 1;
        }
        if (kind == 3) {
            q[9] = q[9] + 1;
            q[1] = q[1] + q[5];
            q[0] = q[0] + q[4];
            q[2] = q[2] + q[6];
            return q[9] < 0x10 ? 0 : 1;
        }
        return 0;
    }
    if (mode != 2)
        return 0;
    if (kind == 0) {
        s2 = func_80077DC4(D_800E27EC << 10) / 8 + 0x1000;
        s2 = (s2 * 3) / 4;
        D_800F336A = 2;
        D_800F3368 = 0x20;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        b.h[2] = (short)(D_800E27EC << 8);
        i = CLUT(0x20, c);
        func_800CEE20(q, b.b, s2, s2, 0xA0, i, 0, 0xBE, 0);
        D_800F3368 = 0x10;
        D_800F3376 = 0x10;
        D_800F3378 = 0x10;
        D_800F336A = 1;
        ang.vx = 0x400;
        ang.vy = 0;
        ang.vz = 0;
        ang.pad = 1;
        pos[0] = q[0];
        pos[1] = D_800942EC;
        pos[2] = q[2];
        i = CLUT(0x10, c);
        func_800CEE20(pos, &ang, s2 * 2, s2 * 2, D_800F336A * 3 + 0xA7, i, 2, 0x64, 0);
        return 0;
    }
    if (kind == 1) {
        s2 = func_80077DC4(q[9] << 6) + 0x1000;
        s0 = q[9] / 2;
        if (s0 >= 7)
            s0 = 6;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        i = CLUT(0, c);
        func_800CEE20(q, a.b, (s2 * 3) / 2, (s2 * 3) / 2, D_800F336A * s0, i, 0, 0xBE, 0);
        return 0;
    }
    if (kind == 2) {
        s2 = func_80077DC4(q[9] << 7) / 2 + 0x800;
        s0 = func_80077DC4(q[9] << 7) / 32;
        D_800F3368 = 0x10;
        D_800F336A = 1;
        D_800F3376 = 0x10;
        D_800F3378 = 0x10;
        i = CLUT(0x30, c);
        func_800CEE20(q, 0, s2, s2, 0xA2, i, 1, s0, 0);
        return 0;
    }
    if (kind == 3) {
        s2 = func_80077CF4((q[9] << 10) / 24) * 2 + 0x1000;
        s0 = func_80077DC4((q[9] * 1204) / 24) / 32;
        D_800F3368 = 0x20;
        D_800F336A = 2;
        D_800F3376 = 0x20;
        D_800F3378 = 0x20;
        i = CLUT(0x10, c);
        func_800CEE20(q, 0, 0x1000, s2, D_800F336A * (short)(q[9] / 2) + 0x20, i, 1, s0, 0);
        return 0;
    }
    return 0;
}
