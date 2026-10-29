/* NON_MATCHING: functionally equivalent to retail room_m0231i 0x3560..0x3958
 * (func_80192548).  Written from the retail asm, not byte-matched; native port
 * differential-tested by tools/analysis/e21_diff.py.
 * Museum 2F (m0231i / m0234i) enemy spore-cloud particle callback.  The
 * particle (q) carries a ring: 0x10 heading, 0x12 swirl phase, 0x14 radius,
 * 0x16 swirl amplitude, 0x08.. the drawn point; v = per-frame deltas
 * (radius, amplitude, phase).  D_800E27EC is the particle age.  Mode 1 swirls
 * the point (func_800CF844) and poisons the player once in range (100) while
 * the owner is in its attack pose; retires at age 64.  Mode 2 draws the cloud
 * and its floor shadow. */
typedef struct {
    short vx, vy, vz, pad;
} SVec;

extern int *D_800F32D0;
extern int *D_8009D254;
extern short D_800942EC;
extern unsigned char *D_800E2368;
extern int D_800E27EC;
extern unsigned char D_8018F1CC[4];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern int func_80077CF4(int a);
extern int func_80077DC4(int a);
extern int func_80077AA4(int a0, int a1);
extern void func_800CF844(void *q, void *pt, int r, SVec *ang, int y, int a5);
extern int func_800C6B90(void *pos, int r);
extern void func_800CEE20(void *pos, void *ang, int sx, int sy, int a4, int clut, int abr, int bright, void *extra);

int func_80192548(int mode, unsigned char *q, int *v)
{
    struct {
        SVec ang;
        unsigned char c[4];
    } st; /* retail frame order: sp+0x28 / 0x30 */
    short *h = (short *)q;
    unsigned char *w;
    int t, s1, s2, k, cl, *o;

    for (k = 0; k < 4; k++)
        st.c[k] = D_8018F1CC[k];
    if (mode == 1) {
        h[10] = h[10] + v[0];
        h[11] = h[11] + v[1];
        h[9] = h[9] + v[2];
        t = func_80077CF4(h[9]) * h[11] / 4096;
        st.ang.vx = 0;
        st.ang.vy = h[8];
        st.ang.vz = 0;
        func_800CF844(q, q + 8, h[10], &st.ang, t, 0);
        if (func_800C6B90(q + 8, 100) && D_800E2368[13] != 0) {
            w = (unsigned char *)D_800F32D0[2];
            if ((**(int **)w & 0x3F000000) == 0x01000000) {
                ((int *)D_8009D254[0])[0x13] |= 0x4000;
                o = *(int **)w;
                *o = (*o & 0xC0FFFFFF) | 0x21000000;
                o = *(int **)w;
                *o |= 0x80000000;
            }
        }
        return D_800E27EC < 64 ? 0 : 1;
    }
    if (mode != 2)
        return 0;
    s2 = func_80077DC4(D_800E27EC << 4) / 32;
    s2 = h[7] * s2 / 128;
    s1 = (D_800E27EC << 8) + 4096;
    s1 = (func_80077DC4(h[3]) / 4 + 4096) * s1 / 4096;
    cl = D_800E1204[D_800F336C] + ((D_800F336C == 4 && D_800F3428) ? 4 : 0);
    cl = func_80077AA4(128, cl) & 0xFFFF;
    func_800CEE20(q + 8, 0, s1, s1, D_800F336A * ((D_800E27EC / 4) & 1) + 253, cl, 1, s2, st.c);
    st.ang.vx = 1024;
    st.ang.vy = 0;
    st.ang.vz = 0;
    st.ang.pad = 1;
    h[5] = D_800942EC;
    cl = D_800E1204[D_800F336C] + ((D_800F336C == 4 && D_800F3428) ? 4 : 0);
    cl = func_80077AA4(128, cl) & 0xFFFF;
    func_800CEE20(q + 8, &st.ang, s1, s1, D_800F336A * ((D_800E27EC / 4) & 1) + 253, cl, 2, s2 / 4, 0);
    return 0;
}
