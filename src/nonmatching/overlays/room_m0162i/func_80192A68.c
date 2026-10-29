/* NON_MATCHING: functionally equivalent to retail room_m0162i 0x3A80..0x4130
 * (func_80192A68; the same bytes run in m0163i).  Written from the retail asm,
 * not byte-matched; native port differential-tested by tools/analysis/e21_diff.py.
 * Battle effect (enemy shot): mode 0 aims it from the source, plays the shot
 * sound and starts a 24-particle pool (func_801924FC); mode 1 steps a
 * charge (16 frames) / flight (4) / impact (4) timeline and spawns the two
 * trail particles; mode 2 draws it (glow sprite, then the impact meshes). */
typedef struct {
    short vx, vy, vz, pad;
} SVec;
typedef struct {
    int m[8];
} Mtx;

extern SVec D_8018F1D8;
extern int *D_800F32D0;
extern int *D_800F33E0;
extern unsigned char *D_800E2368;
extern short D_800942EC;
extern unsigned short D_800E11EC;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800F3368, D_800F336A, D_800F336E, D_800F3370;
extern short D_800F3372, D_800F3374, D_800F3376, D_800F3378;
extern unsigned short D_800F336C;
extern Mtx *D_800BCFA4;
extern unsigned char D_80193234[];
extern void func_800CE9D4(int pool, int a1, short *dir);
extern int func_800D3FD8(void);
extern int func_800D3F64(int id, int pan);
extern void func_800CE8F0(int pool, int idx, SVec *ang, void *out);
extern void func_800CFB7C(short *dir, int dist, SVec *out);
extern int func_800CE560(int pool, int n, int life, void *fn);
extern short *func_800CE610(int pool);
extern int func_80077CF4(int a);
extern int func_80077AA4(int a0, int a1);
extern void func_800CEE20(SVec *pos, SVec *ang, int sx, int sy, int tex, int clut,
                          int abr, int bright, int color);
extern void func_800783E4(SVec *out, short *q, int wa, int wb, SVec *res);
extern void func_800CF3AC(unsigned char *tbl, SVec *m, int frame);
extern void func_800D2B58(void *a0, void *a1, SVec *m0, SVec *m1, int a4, int a5, int a6);
extern void func_801924FC();

int func_80192A68(int mode, short *q, int *src)
{
    SVec ang;
    SVec tmp;
    SVec res;
    SVec m;
    Mtx **slot;
    short *r;
    int k, s, b, clut;
    int *p;
    unsigned char *flag;

    ang = D_8018F1D8;
    if (mode == 0) {
        func_800CE9D4(D_800F32D0[2], 0, q + 4);
        q[11] = -1;
        k = *(short *)(D_800E2368 + 18);
        if (k == 0) {
            q[10] = 32;
            q[5] -= src[2];
            q[11] = func_800D3F64(1421, func_800D3FD8());
        } else if (k == 1) {
            q[10] = 22;
        } else if (k == 2) {
            q[10] = 27;
            q[5] += src[2];
        }
        func_800CE8F0(D_800F32D0[2], q[10], &ang, q);
        q[1] = D_800942EC;
        func_800CFB7C(q + 4, 300, &tmp);
        q[0] += tmp.vx;
        q[1] += tmp.vy;
        q[2] += tmp.vz;
        if (D_800E2368[13]) {
            p = (int *)D_800F32D0[2];
            if (p != 0 && *p != 0) {
                flag = *(unsigned char **)(*p + 24);
                if (*flag == 1) {
                    *flag = 2;
                }
            }
        }
        q[8] = 0;
        q[9] = 0;
        return func_800CE560(D_800F33E0[2], 24, 16, func_801924FC);
    }
    if (mode == 1) {
        k = q[8];
        if (k == 0) {
            q[9]++;
            if (q[9] >= 16) {
                q[8] = 1;
                q[9] = 0;
            }
            return 0;
        }
        if (k == 1) {
            q[9]++;
            if (q[9] < 4) {
                return 0;
            }
            q[8] = 2;
            q[9] = 0;
            r = func_800CE610(D_800F33E0[2]);
            if (r != 0) {
                r[0] = q[0];
                r[1] = q[1];
                r[2] = q[2];
                r[5] = q[5];
                r[4] = 0;
                r[8] = 0;
                r[9] = 0;
                r[6] = q[6];
                r[10] = q[11];
            }
            r = func_800CE610(D_800F33E0[2]);
            if (r != 0) {
                r[0] = q[0];
                r[1] = q[1];
                r[8] = 1;
                r[9] = 0;
                r[2] = q[2];
            }
            return 0;
        }
        if (k == 2) {
            q[9]++;
            return q[9] < 4 ? 0 : 2;
        }
        return 0;
    }
    if (mode != 2) {
        return 0;
    }
    func_800CE8F0(D_800F32D0[2], q[10], &ang, &tmp);
    D_800F3368 = 32;
    D_800F336A = 2;
    D_800F3376 = 32;
    D_800F3378 = 32;
    D_800F336C = 4;
    D_800F336E = 0;
    D_800F3372 = 0;
    D_800F3374 = 12;
    D_800F3370 = D_800E2850[D_800E11EC];
    k = q[8];
    if (k == 0) {
        func_80077CF4(q[9] << 6);
        b = (q[9] & 1) ? 64 : 128;
        s = func_80077CF4(q[9] << 6);
        clut = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428) {
            clut += 4;
        }
        func_800CEE20(&tmp, 0, s, s, 32, func_80077AA4(16, clut) & 0xFFFF, 1, b, 0);
        return 0;
    }
    if (k == 1) {
        func_800783E4(&tmp, q, 4096 - (q[9] << 10), q[9] << 10, &res);
        func_800CF3AC(D_80193234, &m, q[9]);
        slot = &D_800BCFA4;
        asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( *slot ) : "$12", "$13", "$14") ;
        asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( *slot ) : "$12", "$13", "$14") ;
        func_800D2B58(&tmp, &res, &m, &m, 0, 128, 1);
        return 0;
    }
    if (k == 2 && q[9] < 5) {
        func_800CF3AC(D_80193234, &m, q[9] + 4);
        slot = &D_800BCFA4;
        asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( *slot ) : "$12", "$13", "$14") ;
        asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( *slot ) : "$12", "$13", "$14") ;
        func_800D2B58(&tmp, q, &m, &m, 0, 128, 1);
    }
    return 0;
}
