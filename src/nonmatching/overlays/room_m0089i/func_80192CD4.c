/* NON_MATCHING: functionally equivalent to retail room_m0089i 0x3CEC..0x4218
 * (func_80192CD4).  Written from the retail asm, not byte-matched; native port
 * differential-tested by tools/analysis/e21_diff.py.
 * Battle effect callback (Day 2 Central Park random battles, m0089i): mode 0
 * creates the effect model (func_8006E498) and resets the timer q[0]/phase
 * q[1]; mode 1 ticks the timer (finished once the phase reaches 2); mode 2
 * draws: a growing flash sprite (phase 0/1) at the position a2, then the
 * model twice (scale s*2/3 and s/3) above the floor, raised by the frame
 * counter D_800E27EC. */
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
extern unsigned char D_8018F1D4[4];
extern int D_80194168;
extern int D_800B0E64;
extern int D_800E27EC;
extern int D_800F3428;
extern unsigned short D_800E11FA;
extern unsigned short D_800E120A;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern short D_800F3368, D_800F336A, D_800F336C, D_800F336E, D_800F3370;
extern short D_800F3372, D_800F3374, D_800F3376, D_800F3378;
extern int func_8006E498();
extern void func_800C6D5C();
extern int func_80077DC4();
extern int func_80077CF4();
extern int func_80077AA4();
extern int func_80077A64();
extern void func_800CEE20();
extern void func_800C6EC0();
extern void func_800C6ED8();
extern void func_80079754();
extern void func_80078CC4();
extern void func_800C6EF8();
extern void func_800C6FA0();
extern void func_800C71E4();
extern void func_800C6F4C();

int func_80192CD4(int mode, short *q, short *a2)
{
    struct { SVECTOR pos; SVECTOR ang; unsigned char extra[4]; } f; /* retail sp+40/48/56 */
    MATRIX m1;
    VECTOR s1v;
    MATRIX m2;
    VECTOR s2v;
    int i;
    int sz;
    int br;
    int sc;
    int fr;
    int t;
    int c;
    int k;

    for (i = 0; i < 4; i++)
        f.extra[i] = D_8018F1D4[i];
    if (mode == 1) {
        q[0] = q[0] + 1;
        if (q[1] < 2)
            return 0;
        return 1;
    }
    if (mode == 0) {
        q[1] = 0;
        q[0] = 0;
        D_80194168 = func_8006E498(D_800B0E64, 0xC5864704, a2);
        func_800C6D5C(D_80194168, 0, 0);
        return 0;
    }
    if (mode != 2)
        return 0;
    sz = 0;
    br = 0;
    sc = 0;
    fr = 0;
    if (q[1] == 0) {
        sc = func_80077DC4(q[0] << 6);
        fr = func_80077CF4(q[0] << 7) / 32;
        br = 64;
        sz = func_80077CF4(q[0] << 6);
        if (q[0] >= 16) {
            q[1] = 1;
            q[0] = 0;
        }
    } else if (q[1] == 1) {
        t = func_80077DC4(q[0] << 6) / 32;
        br = (t * 3) / 2;
        sz = 4096;
        if (q[0] >= 16) {
            q[1] = 2;
            q[0] = 0;
        }
    }
    D_800F3368 = 32;
    D_800F336A = 2;
    D_800F3376 = 32;
    D_800F3378 = 32;
    D_800F336C = 3;
    D_800F336E = 1;
    D_800F3372 = 0;
    D_800F3374 = 2;
    D_800F3370 = D_800E2850[D_800E11FA];
    f.pos.vx = a2[0];
    f.pos.vy = a2[1];
    f.pos.vz = a2[2];
    if (sz != 0) {
        f.ang.vx = 1024;
        f.ang.vy = 0;
        f.ang.pad = 1;
        f.ang.vz = -D_800E27EC * 64;
        c = func_80077AA4(0, D_800E120A + 5) & 0xFFFF;
        func_800CEE20(&f.pos, &f.ang, sz, sz, 38, c, 1, br, f.extra);
    }
    f.ang.vx = 0;
    f.ang.vz = 0;
    f.pos.vy = f.pos.vy - (32 - D_800E27EC) * 16;
    sc = (sc * 2) / 3;
    f.ang.vy = -(D_800E27EC << 6);
    D_800F336C = 3;
    D_800F336E = 1;
    D_800F3370 = D_800E2850[D_800E11FA];
    k = (D_800E2850[D_800E11FA] | func_80077A64(0, 1, 0, 0)) & 0xFFFF;
    c = D_800E1204[(unsigned short)D_800F336C];
    if ((unsigned short)D_800F336C == 4 && D_800F3428 != 0)
        c += 7;
    else
        c += 3;
    func_800C6EC0(k, func_80077AA4(0, c) & 0xFFFF);
    func_800C6ED8(1);
    func_80079754(&f.ang, &m1);
    s1v.vx = sc;
    s1v.vy = sc;
    s1v.vz = sc;
    m1.t[0] = f.pos.vx;
    m1.t[1] = f.pos.vy;
    m1.t[2] = f.pos.vz;
    func_80078CC4(&m1, &s1v);
    func_800C6EF8(D_80194168);
    func_800C6FA0(D_80194168, (unsigned short)fr);
    func_800C71E4(D_80194168, &m1);
    func_800C6F4C(D_80194168);
    f.pos.vy = f.pos.vy + (32 - D_800E27EC) * 8;
    func_80079754(&f.ang, &m2);
    s2v.vx = sc / 2;
    s2v.vy = sc / 2;
    s2v.vz = sc / 2;
    m2.t[0] = f.pos.vx;
    m2.t[1] = f.pos.vy;
    m2.t[2] = f.pos.vz;
    func_80078CC4(&m2, &s2v);
    func_800C6EF8(D_80194168);
    func_800C6FA0(D_80194168, (unsigned short)fr);
    func_800C71E4(D_80194168, &m2);
    func_800C6F4C(D_80194168);
    return 0;
}
