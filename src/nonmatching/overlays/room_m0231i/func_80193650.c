/* NON_MATCHING: functionally equivalent to retail room_m0231i 0x4668..0x5444
 * (func_80193650).  Written from the retail asm, not byte-matched; native port
 * differential-tested by tools/analysis/e21_diff.py.
 * Museum 2F (m0231i) enemy acid-spit particle callback.  q[0..2] position,
 * q[4..6] velocity / heading, q[8] state, q[9] age; a2 points at the heading.
 * Mode 1: state 0 flies (func_800CFB7C step, trail droplets every other frame)
 * and on a hit (func_800C6B90 within 50, above floor - 514) poisons the player
 * when the owner is in its spit pose, then bursts into 6 droplets (state 3) and
 * becomes the splash (state 2); it also splashes on the floor or a wall
 * (func_8001CAB0).  States 1 / 3 are falling / rising droplets (bounce on the
 * floor, 6 / 12 frames), 2 / 4 the 16-frame splash.  Mode 2 draws each state. */
typedef struct {
    short vx, vy, vz, pad;
} SVec;

extern int *D_800F33E0;
extern int *D_800F32D0;
extern int *D_8009D254;
extern int D_8009D248;
extern unsigned short D_8009D1CC;
extern short D_800942EC;
extern unsigned char *D_800E2368;
extern int *D_800BCFA4;
extern unsigned char D_8018F1EC[4], D_8018F1F0[4];
extern unsigned short D_800E1204[], D_800E2850[];
extern unsigned short D_800E1208, D_800E11E8, D_800E120C, D_800E11EC;
extern int D_800F3428;
extern short D_800F3368, D_800F336A, D_800F336E, D_800F3370;
extern unsigned short D_800F336C;
extern volatile short D_800F3376, D_800F3378;
extern unsigned char *func_800CE610(int pool);
extern int func_80071A54(void);
extern void func_800CFB7C(short *heading, int speed, SVec *out);
extern int func_800C6B90(short *pos, int r);
extern int func_8001CAB0(int x, int z, int a2, int a3);
extern int func_80077AA4(int a0, int a1);
extern int func_80077DC4(int a);
extern int func_80077CF4(int a);
extern void func_800CEE20(void *pos, void *ang, int sx, int sy, int a4, int clut, int abr, int bright, void *extra);
extern void func_800D004C(void *pos, int a1, int a2, int a3, void *ang, int sx, int sy, void *a7, void *a8, int a9, int a10);

#define BLEND16()                                                      \
    do {                                                               \
        D_800F3368 = 16;                                               \
        D_800F336A = 1;                                                \
        D_800F3376 = 16;                                               \
        D_800F3378 = 16;                                               \
        D_800F336C = 2;                                                \
        D_800F336E = 0;                                                \
        D_800F3370 = D_800E2850[D_800E11E8];                           \
    } while (0)
#define CLUT_SEL() (D_800E1204[D_800F336C] + ((D_800F336C == 4 && D_800F3428) ? 4 : 0))

int func_80193650(int mode, short *q, short *hd)
{
    struct {
        SVec v;
        unsigned char c0[4], g0[4], c1[4], g1[4];
        SVec pos, ang;
    } st; /* retail frame order: sp+0x30 / 0x38 / 0x40 / 0x48 / 0x50 */
    short *p;
    int i, k, r, s2, s3, c, *o, *m;
    unsigned char *w;

    for (i = 0; i < 4; i++) {
        st.c0[i] = D_8018F1EC[i];
        st.c1[i] = D_8018F1F0[i];
    }
    if (mode == 1) {
        switch (q[8]) {
        case 0:
            q[9] = q[9] + 1;
            func_800CFB7C(q + 4, hd[0], &st.v);
            q[0] = q[0] + st.v.vx;
            q[1] = q[1] + st.v.vy;
            q[2] = q[2] + st.v.vz;
            if (!(q[9] & 1)) {
                p = (short *)func_800CE610(D_800F33E0[2]);
                if (p) {
                    p[0] = q[0];
                    p[1] = q[1];
                    p[2] = q[2];
                    r = func_80071A54();
                    p[4] = r - r / 32 * 32 - 16;
                    r = func_80071A54();
                    p[5] = r - r / 32 * 32 - 16;
                    r = func_80071A54();
                    p[6] = r - r / 32 * 32 - 16;
                    p[8] = 1;
                    p[9] = 0;
                }
            }
            if (func_800C6B90(q, 50) && q[1] > D_800942EC - 514) {
                if (D_800E2368[13] != 0) {
                    w = (unsigned char *)D_800F32D0[2];
                    if ((**(int **)w & 0x3F000000) == 0x01000000) {
                        ((int *)D_8009D254[0])[0x13] |= 0x4000;
                        o = *(int **)w;
                        *o = (*o & 0xC0FFFFFF) | 0x19000000;
                        o = *(int **)w;
                        *o |= 0x80000000;
                    }
                }
                q[8] = 2;
                q[9] = 0;
                for (k = 0; k < 6; k++) {
                    p = (short *)func_800CE610(D_800F33E0[2]);
                    if (!p)
                        continue;
                    p[0] = q[0];
                    p[1] = q[1];
                    p[2] = q[2];
                    r = func_80071A54();
                    p[4] = r - r / 40 * 40 - 20;
                    r = func_80071A54();
                    p[5] = r - r / 40 * 40 - 20;
                    p[8] = 3;
                    p[9] = 0;
                    r = func_80071A54();
                    p[6] = r - r / 40 * 40 - 20;
                }
            }
            if (q[1] >= D_800942EC) {
                q[8] = 2;
                q[9] = 0;
                for (k = 0; k < 6; k++) {
                    p = (short *)func_800CE610(D_800F33E0[2]);
                    if (!p)
                        continue;
                    p[0] = q[0];
                    p[1] = q[1];
                    p[2] = q[2];
                    r = func_80071A54();
                    p[4] = r - r / 40 * 40 - 20;
                    r = func_80071A54();
                    p[5] = r - r / 40 * 40 - 20;
                    p[8] = 3;
                    p[9] = 0;
                    r = func_80071A54();
                    p[6] = r - r / 40 * 40 - 20;
                }
            }
            if (func_8001CAB0(q[0] << 16, q[2] << 16, D_8009D248, D_8009D1CC))
                return 0;
            q[8] = 2;
            q[9] = 0;
            return 0;
        case 1:
        case 3:
            k = q[4];
            q[9] = q[9] + 1;
            q[0] = q[0] + q[4];
            q[1] = q[1] + q[5];
            q[2] = q[2] + q[6];
            q[4] = (k * 63) / 64;
            q[6] = (q[6] * 63) / 64;
            q[5] = q[5] + (q[8] == 1 ? -1 : 1);
            if (q[1] >= D_800942EC)
                q[5] = -q[5];
            return q[9] < (q[8] == 1 ? 6 : 12) ? 0 : 1;
        case 2:
        case 4:
            q[9] = q[9] + 1;
            return q[9] < 16 ? 0 : 1;
        }
        return 0;
    }
    if (mode != 2)
        return 0;
    m = D_800BCFA4;
    asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"(m) : "$12", "$13", "$14");
    asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"(m) : "$12", "$13", "$14");
    switch (q[8]) {
    case 0:
        st.v.vx = 0;
        st.v.vy = 0;
        st.v.vz = q[9] << 7;
        st.v.pad = 0;
        BLEND16();
        s2 = 128 - ((q[9] & 1) << 6);
        c = func_80077AA4(32, D_800E1208) & 0xFFFF;
        func_800CEE20(q, &st.v, 4096, 4096, 216, c, 1, s2, st.c0);
        c = func_80077AA4(32, CLUT_SEL()) & 0xFFFF;
        func_800CEE20(q, &st.v, 8192, 8192, D_800F336A + 216, c, 1, s2 / 2, st.c1);
        st.ang.vx = 1024;
        st.ang.vy = 0;
        st.ang.vz = 0;
        st.ang.pad = 1;
        st.pos.vx = q[0];
        st.pos.vy = D_800942EC;
        st.pos.vz = q[2];
        c = func_80077AA4(32, CLUT_SEL()) & 0xFFFF;
        func_800CEE20(&st.pos, &st.ang, 4096, 4096, D_800F336A + 216, c, 1, s2, st.c1);
        break;
    case 1:
        s2 = func_80077CF4(q[9] << 7) / 64 + 64;
        BLEND16();
        c = func_80077AA4(32, D_800E1208) & 0xFFFF;
        func_800CEE20(q, 0, 4096, 4096, D_800F336A * q[9] + 224, c, 1, s2, st.c1);
        break;
    case 2:
        s2 = func_80077DC4(q[9] << 6) / 32;
        if (q[9] & 1)
            s2 = (s2 * 2) / 3;
        D_800F3368 = 32;
        D_800F336A = 2;
        D_800F3376 = 32;
        D_800F3378 = 32;
        D_800F336C = 4;
        D_800F336E = 0;
        D_800F3370 = D_800E2850[D_800E11EC];
        c = func_80077AA4(0, D_800E120C + (D_800F3428 ? 4 : 0)) & 0xFFFF;
        func_800CEE20(q, 0, 6144, 6144, D_800F336A * (q[9] / 2) + 192, c, 1, s2, st.c0);
        s3 = func_80077CF4(q[9] << 6) + 2048;
        func_800D004C(q, 300, 300, 5, 0, s3, s3, st.c1, 0, s2 / 2, 1);
        break;
    case 3:
        s2 = func_80077DC4((q[9] << 10) / 12) / 32;
        BLEND16();
        c = func_80077AA4(32, D_800E1208) & 0xFFFF;
        func_800CEE20(q, 0, 4096, 4096, D_800F336A * ((q[9] * 2) / 3) + 200, c, 1, s2, st.c1);
        break;
    case 4:
        s3 = func_80077CF4(q[9] << 6) + 4096;
        s2 = func_80077DC4(q[9] << 6) / 32;
        q[7] = 1;
        q[6] = q[9] * 24;
        BLEND16();
        D_800F336A = 1;
        c = func_80077AA4(144, D_800E1208) & 0xFFFF;
        func_800CEE20(q, q + 4, s3, s3, D_800F336A * 2 + 253, c, 1, s2, 0);
        break;
    }
    return 0;
}
