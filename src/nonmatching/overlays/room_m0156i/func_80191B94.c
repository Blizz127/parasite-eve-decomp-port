/* NON_MATCHING: functionally equivalent to retail room_m0156i 0x2BAC..0x3220
 * (func_80191B94; the same bytes run in m0380i).  Written from the retail asm,
 * not byte-matched; native port differential-tested by tools/analysis/e21_diff.py.
 * Per-particle callback of func_80192208's debris pool.  q[0..2] position,
 * q[3] spin, q[4..6] velocity, q[8] kind (0 dust, 1/2 sparks), q[9] age.
 * Mode 1 integrates (drag, bounce off the floor height D_800942EC, jitter) and
 * returns 1 when the particle has expired; mode 2 draws it. */
typedef struct {
    short vx, vy, vz, pad;
} SVec;

extern SVec D_8018F050;
extern short D_800942EC;
extern unsigned short D_800F336C;
extern short D_800F336A;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_80071A54(void);
extern int func_80077DC4(int a);
extern int func_80077AA4(int a0, int a1);
extern void func_800CEE20(short *pos, SVec *ang, int sx, int sy, int tex, int clut,
                          int abr, int bright, int color);

int func_80191B94(int mode, short *q)
{
    SVec ang;
    int k, s, b, v, clut, tex;

    ang = D_8018F050;
    if (mode == 1) {
        k = q[8];
        if (k == 0) {
            q[9]++;
            q[0] += q[4];
            q[1] += q[5];
            q[2] += q[6];
            q[4] = q[4] * 127 / 128;
            q[6] = q[6] * 127 / 128;
            if (q[1] >= D_800942EC) {
                q[5] = -q[5];
            }
            q[0] += (func_80071A54() & 0xF) - 7;
            q[1] += (func_80071A54() & 0xF) - 7;
            q[2] += (func_80071A54() & 0xF) - 7;
            return q[9] < 32 ? 0 : 1;
        }
        if (k < 0 || k >= 3) {
            return 0;
        }
        q[9]++;
        q[0] += q[4];
        q[1] += q[5];
        q[2] += q[6];
        q[4] = q[4] * 63 / 64;
        q[6] = q[6] * 63 / 64;
        if (q[1] >= D_800942EC) {
            q[5] = -q[5];
        }
        q[4] += (func_80071A54() & 0xF) - 7;
        q[5] += (func_80071A54() & 0xF) - 7;
        q[6] += (func_80071A54() & 0xF) - 7;
        return q[9] < 20 ? 0 : 1;
    }
    if (mode != 2) {
        return 0;
    }
    k = q[8];
    if (k == 0) {
        s = q[9] * 32;
        b = func_80077DC4(s) / 60;
        v = func_80077DC4(s) / 4 + 0x1000;
        s = v * 3;
        if (q[3] & 1) {
            s /= 2;
        }
        ang.vz = q[3] + q[9] * 8;
        clut = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428) {
            clut += 4;
        }
        func_800CEE20(q, &ang, s, s, 0x80, func_80077AA4(0x40, clut) & 0xFFFF, 1, b, 0);
        return 0;
    }
    if (k != 1 && k != 2) {
        return 0;
    }
    s = (q[9] << 10) / 20;
    b = k == 1 ? func_80077DC4(s) / 60 : func_80077DC4(s) / 32;
    v = func_80077DC4(s) / 4 + 0x1000;
    s = v * 2;
    if (q[3] & 1) {
        s >>= 1;
    }
    ang.vz = q[3] + q[9] * 16;
    clut = D_800E1204[D_800F336C];
    if (D_800F336C == 4 && D_800F3428) {
        clut += 4;
    }
    clut = func_80077AA4(0x40, clut) & 0xFFFF;
    tex = D_800F336A * (short)(q[9] / 6) + 0x80;
    if (k == 1) {
        func_800CEE20(q, &ang, s, s, tex, clut, 1, b / 2, 0);
    } else {
        func_800CEE20(q, &ang, s, s, tex, clut, 2, b / 4, 0);
    }
    return 0;
}
