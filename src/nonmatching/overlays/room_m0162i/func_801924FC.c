/* NON_MATCHING: functionally equivalent to retail room_m0162i 0x3514..0x3A80
 * (func_801924FC; the same bytes run in m0163i).  Written from the retail asm,
 * not byte-matched; native port differential-tested by tools/analysis/e21_diff.py.
 * Per-particle callback of func_80192A68's shot pool.  q[0..2] position,
 * q[4..6] direction, q[8] kind (0 shot, 1 hit flash, 2 trail), q[9] age,
 * q[10] sound voice.  Mode 1 steers the shot at the player (D_8009D254), on a
 * hit flags the player's body and the shooter, and stops the voice; mode 2 draws. */
typedef struct {
    short vx, vy, vz, pad;
} SVec;

extern unsigned char D_8018F1CC[4];
extern SVec D_8018F1D0;
extern int **D_8009D254;
extern int *D_800F32D0;
extern int *D_800F33E0;
extern unsigned char *D_800E2368;
extern int D_800E27EC;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern void func_800CE870(int **who, int a1, SVec *out);
extern void func_800CFAA8(short *from, SVec *to, SVec *out);
extern void func_800CFD50(SVec *ang, short *dir, int turn);
extern void func_800CFB7C(short *dir, int dist, SVec *out);
extern int func_800C6B90(short *pos, int r);
extern void func_800866A4(int voice, int a1);
extern short *func_800CE610(int pool);
extern int func_80077AA4(int a0, int a1);
extern void func_800CEE20(short *pos, SVec *ang, int sx, int sy, int tex, int clut,
                          int abr, int bright, void *extra);

#define CLUT(a0) (c = D_800E1204[D_800F336C],                        \
                  (D_800F336C == 4 && D_800F3428) ? c += 4 : 0,      \
                  func_80077AA4(a0, c) & 0xFFFF)

int func_801924FC(int mode, short *q, short *src)
{
    struct {
        unsigned char ex[4];
        int pad;
        SVec ang;
    } l; /* retail frame order: ex at sp+40, ang at sp+48 */
    SVec t;
    SVec d;
    short *r;
    int *p;
    int k, b, c;

    l.ex[0] = D_8018F1CC[0];
    l.ex[1] = D_8018F1CC[1];
    l.ex[2] = D_8018F1CC[2];
    l.ex[3] = D_8018F1CC[3];
    l.ang = D_8018F1D0;
    if (mode == 1) {
        k = q[8];
        if (k == 0) {
            func_800CE870(D_8009D254, 0, &d);
            func_800CFAA8(q, &d, &t);
            t.vx = 0;
            func_800CFD50(&t, q + 4, (unsigned short)src[2]);
            func_800CFB7C(q + 4, src[0], &d);
            q[0] += d.vx;
            q[1] += d.vy;
            q[2] += d.vz;
            if (func_800C6B90(q, 50)) {
                if (D_800E2368[13]) {
                    if ((**(int **)D_800F32D0[2] & 0x3F000000) == 0x01000000) {
                        (*D_8009D254)[19] |= 0x4000;
                        p = *(int **)D_800F32D0[2];
                        *p = (*p & 0xC0FFFFFF) | 0x11000000;
                        p = *(int **)D_800F32D0[2];
                        *p |= 0x80000000;
                    }
                }
                q[8] = 1;
                q[9] = 0;
                if (q[10] != -1) {
                    func_800866A4(q[10], 0);
                }
            }
            if (D_800E27EC & 1) {
                r = func_800CE610(D_800F33E0[2]);
                if (r != 0) {
                    r[0] = q[0];
                    r[1] = q[1];
                    r[8] = 2;
                    r[9] = 0;
                    r[2] = q[2];
                }
            }
            if (D_800E27EC < 36) {
                return 0;
            }
            if (q[10] != -1) {
                func_800866A4(q[10], 0);
            }
            return 1;
        }
        if (k == 1) {
            q[9]++;
            q[1] -= 8;
            return q[9] < 8 ? 0 : 1;
        }
        if (k == 2) {
            q[9]++;
            return q[9] < 16 ? 0 : 1;
        }
        return 0;
    }
    if (mode != 2) {
        return 0;
    }
    k = q[8];
    if (k == 0) {
        b = (D_800E27EC & 1) ? 190 : 128;
        func_800CEE20(q, &l.ang, 4096, 4096, 32, CLUT(16), 1, b / 2, 0);
    } else if (k == 1) {
        func_800CEE20(q, &l.ang, 4096, 4096, D_800F336A * q[9], CLUT(0), 1, 128, l.ex);
    } else if (k == 2) {
        b = 41 - q[9] * 41 / 16;
        func_800CEE20(q, &l.ang, 4096, 4096, 32, CLUT(16), 1, b, 0);
    }
    return 0;
}
