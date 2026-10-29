/* NON_MATCHING: functionally equivalent to retail room_m0273i 0x9EAC..0xA408
 * (func_80198E94).  Written from the retail asm, not byte-matched; native port
 * differential-tested by tools/analysis/e21_diff.py.
 * Eve (m0273i) ground-shock ring particle callback: q[0..2] centre, q[3] hit
 * latch; D_800E27EC is the particle age.  Mode 1 hits the player once while
 * the pulsing ring radius covers them (frames < 0x19, flag 0x4000, 40-frame
 * lockout), retiring at 0x28.  Mode 2 draws the rotating shock plate
 * (D_8019AF6C model, frames 5..36), the flare and two rings. */
typedef struct {
    short m[3][3];
    int t[3];
} Mat;
typedef struct {
    short vx, vy, vz, pad;
} SVec;
typedef struct {
    int vx, vy, vz;
} LVec;

extern int *D_800F32D0;
extern int *D_8009D254;
extern int D_800E27EC;
extern short D_8019AFA0;
extern int D_8019AF6C;
extern unsigned char D_800966EC[];
extern unsigned short D_800E2850[], D_800E1204[];
extern unsigned short D_800E11FA, D_800E11EA, D_800E120A;
extern unsigned short D_800F336C;
extern int D_800F3428;
extern short D_800F3368, D_800F336A, D_800F336E, D_800F3370;
extern volatile short D_800F3376, D_800F3378;
extern unsigned char D_8019AE1C[];
extern int func_8005186C(int sq);
extern int func_80077A64(int a0, int a1, int a2, int a3);
extern int func_80077AA4(int a0, int a1);
extern void func_800C6EC0(int a0, int a1);
extern void func_800C6ED8(int a0);
extern void func_800C6EF8(int a0);
extern void func_800C6FA0(int a0, int a1);
extern void func_800C71E4(int a0, Mat *m);
extern void func_800C6F4C(int a0);
extern int func_80077DC4(int a);
extern int func_80077CF4(int a);
extern void func_80071A44(void *p, int c, int n);
extern void func_80078CC4(Mat *m, LVec *v);
extern void func_800CEE20(short *pos, void *a1, int sx, int sy, int a4, int clut, int abr, int bright, void *extra);
extern void func_800D004C(short *pos, int a1, int a2, int a3, SVec *ang, int sx, int sy, void *a7, void *a8, int a9, int a10);

#define SIN4(i) (*(short *)(D_800966EC + ((i) & 0xFFF) * 4))
#define COS4(i) (*(short *)(D_800966EC + ((i) & 0xFFF) * 4 + 2))

int func_80198E94(int mode, short *q)
{
    struct {
        SVec ang;
        Mat m;
        LVec sc;
    } st; /* retail frame order: sp+0x30 / 0x38 / 0x58 */
    int t, s, r, dx, dz, k, s1, s3, s4, s5, c, a, b, i, *o;

    if (mode == 1) {
        t = D_800E27EC;
        if (t >= 0x28)
            return 1;
        if (t >= 0x19 || q[3] != 0 || D_8019AFA0 != 0)
            return 0;
        s = SIN4((t << 11) / 40);
        r = s / 16 + 0x80;
        dx = D_8009D254[0x7F] - q[0];
        dz = D_8009D254[0x81] - q[2];
        if (r < func_8005186C(dx * dx + dz * dz))
            return 0;
        q[3] = 1;
        ((int *)D_8009D254[0])[0x13] |= 0x4000;
        o = *(int **)D_800F32D0[2];
        if (o) {
            *o |= 0x80000000;
            D_8019AFA0 = 0x28;
        }
        return 0;
    }
    if (mode != 2)
        return 0;
    t = D_800E27EC;
    s5 = t - 1;
    if ((unsigned)(t - 5) < 0x20) {
        k = t - 5;
        if (k < 8)
            s1 = *(short *)(D_800966EC + ((k << 9) & 0x3E00));
        else
            s1 = COS4(((t - 0xD) << 10) / 24);
        a = func_80077A64(0, 1, 0, 0);
        s4 = s1 >> 6;
        s = (D_800E2850[D_800E11FA] | a) & 0xFFFF;
        s3 = s1 >> 2;
        c = D_800E1204[D_800F336C];
        c += (D_800F336C == 4 && D_800F3428) ? 6 : 2;
        c = func_80077AA4(0, c);
        func_800C6EC0(s, c & 0xFFFF);
        func_800C6ED8(1);
        func_800C6EF8(D_8019AF6C);
        func_800C6FA0(D_8019AF6C, s4 & 0xFFFF);
        a = func_80077DC4(s5 << 8);
        b = func_80077CF4(s5 << 8);
        st.m.m[0][0] = a;
        st.m.m[0][1] = 0;
        st.m.m[0][2] = b;
        st.m.m[1][0] = 0;
        st.m.m[1][1] = 0x1000;
        st.m.m[1][2] = 0;
        st.m.m[2][0] = -b;
        st.m.m[2][1] = 0;
        st.m.m[2][2] = a;
        st.m.t[0] = 0;
        st.m.t[1] = 0;
        st.m.t[2] = 0;
        func_80071A44(&st.sc, 0, 0x10);
        st.sc.vx = s3;
        st.sc.vy = s3;
        st.sc.vz = s3;
        func_80078CC4(&st.m, &st.sc);
        st.m.t[0] = q[0];
        st.m.t[1] = q[1];
        st.m.t[2] = q[2];
        func_800C71E4(D_8019AF6C, &st.m);
        func_800C6F4C(D_8019AF6C);
    }
    D_800F3368 = 0x20;
    D_800F336A = 2;
    D_800F3376 = 0x20;
    D_800F3378 = 0x20;
    D_800F3376 = 0x20;
    D_800F3378 = 0x20;
    D_800F336C = 3;
    D_800F336E = 0;
    D_800F3370 = D_800E2850[D_800E11EA];
    if (s5 < 8) {
        s3 = *(short *)(D_800966EC + ((s5 << 9) & 0x3E00)) + 0x1800;
        c = func_80077AA4(0x10, D_800E120A) & 0xFFFF;
        func_800CEE20(q, 0, s3, s3, D_800F336A * s5 + 0x80, c, 1, 0x80, 0);
    }
    st.ang.vx = 0x400;
    st.ang.vy = (-s5) << 9;
    st.ang.vz = 0;
    st.ang.pad = 1;
    s4 = COS4(s5 * 25) >> 5;
    s1 = SIN4(s5 * 51) / 16;
    for (i = 0; i < 2; i++, s1 += 0x100)
        func_800D004C(q, s1, s1, 8, &st.ang, 0x1000, 0x1000, D_8019AE1C, D_8019AE1C + 4, s4, 1);
    return 0;
}
