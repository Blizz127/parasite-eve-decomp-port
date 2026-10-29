/* NON_MATCHING native input: room_m0086i 0x74C..0x10E4 (0x998 bytes).
 * Code-0x78 effect callback. The guest C below translates the retail sequence
 * documented in pc_port/game/boot/room_m0086i_F7_port.c in the integrated
 * later-days checkout. Retail assembly remains the exact-rebuild source.
 * q[0..2] is position, q[4..6] velocity, q[8..10] joint direction,
 * q[12] phase and q[13] timer. Phase 0 flies, emits particles and impacts the
 * floor; phase 1 draws the impact for eight ticks; phase 2 expires at 64.
 * Mode 0 returns the pool initializer's actual result. Other modes return
 * the retail completion flag. No missing path is replaced with a stub.
 * Differential verification is required before integration. */

typedef struct { short x, y, z, pad; } SVec;
typedef struct { short m[3][3]; int t[3]; } Matrix;
typedef struct { int x, y, z, pad; } Vector;
typedef struct { int words[8]; } Camera;

extern unsigned char D_8018EFF4[8];
extern unsigned char D_8018EFFC[8];
extern Camera *D_800BCFA4;
extern int *D_800F32D0;
extern int *D_800F33E0;
extern unsigned char *D_800E2368;
extern void *D_800B0E64;
extern int D_800E27EC;
extern short D_800942EC;
extern unsigned short D_800E11E8, D_800E11EA;
extern unsigned short D_800E1204[], D_800E1208, D_800E120A;
extern unsigned short D_800E2850[];
extern short D_800F3368, D_800F336A, D_800F336E, D_800F3372;
extern short D_800F3374, D_800F3376, D_800F3378;
extern unsigned short D_800F336C, D_800F3370;
extern int D_800F3428;
extern void *D_80190B84, *D_80190B88;

extern void *func_8006E498(void *package, unsigned int key);
extern int func_80071A54(void);
extern int func_80077A64(int tp, int abr, int x, int y);
extern int func_80077AA4(int x, int y);
extern int func_80077CF4(int angle);
extern int func_80077DC4(int angle);
extern void func_80078CC4(Matrix *matrix, Vector *scale);
extern void func_80079754(SVec *angles, Matrix *matrix);
extern void func_800C6D5C(void *mesh, int x, int y);
extern void func_800C6EC0(int tpage, int clut);
extern void func_800C6ED8(int value);
extern void func_800C6EF8(void *mesh);
extern void func_800C6FA0(void *mesh, int brightness);
extern void func_800C71E4(void *mesh, Matrix *matrix);
extern void func_800C6F4C(void *mesh);
extern int func_8018F004(int mode, short *particle, int *extra);
extern unsigned int func_800CE560(void *pool, unsigned int stride, int count,
                                 int (*callback)(int, short *, int *));
extern short *func_800CE610(void *pool);
extern void func_800CE8F0(void *actor, unsigned int joint, void *local, void *out);
extern void func_800CE9D4(void *actor, unsigned int joint, void *out);
extern void func_800CEAE8(void *matrix, void *in, void *out);
extern void func_800CEE20(void *position, void *angles, int sx, int sy,
                          int tex, int clut, int abr, int bright, void *extra);
extern int func_800D3F64(unsigned int sound, unsigned int group);
extern int func_800D3FD8(void);

int func_8018F734(int mode, short *q, unsigned int *extra)
{
    SVec angles;                    /* retail sp+0x28 */
    SVec position;                  /* retail sp+0x30 */
    unsigned char joint_local[8];   /* retail sp+0x38 */
    unsigned char local_velocity[8];/* retail sp+0x40 */
    Matrix matrix;                  /* retail sp+0x48 */
    Vector scale;                   /* retail sp+0x68 */
    Camera *camera;
    int *actor, *body;
    unsigned char *flag;
    short *p;
    int i, r, v, speed, angle, phase, size, row, tex, clut, spread;
    unsigned int k, count, step, page;

    for (i = 0; i < 8; i++) joint_local[i] = D_8018EFF4[i];
    for (i = 0; i < 8; i++) local_velocity[i] = D_8018EFFC[i];

    if (mode == 0) {
        /* 8018F7F0..8018F83C: the exact eight camera control-register loads. */
        camera = D_800BCFA4;
        asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t"
                     "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t"
                     "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t"
                     "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t"
                     "ctc2 $13,$3\n\t" "ctc2 $14,$4"
                     : : "r"(camera) : "$12", "$13", "$14");
        asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t"
                     "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t"
                     "ctc2 $13,$6\n\t" "ctc2 $14,$7"
                     : : "r"(camera) : "$12", "$13", "$14");
        func_800CE8F0((void *)D_800F32D0[2], 7, joint_local, q);
        actor = (int *)D_800F32D0[2];
        func_800CEAE8((void *)actor[142], local_velocity, q + 4);
        v = (unsigned short)q[5];
        actor = D_800F32D0;
        q[12] = 0;
        q[13] = 0;
        q[5] = v - 24;
        func_800CE9D4((void *)actor[2], 0, q + 8);
        if (D_800E2368[13] != 0) {
            actor = (int *)D_800F32D0[2];
            if (actor != 0) {
                body = (int *)actor[0];
                if (body != 0) {
                    flag = (unsigned char *)body[6];
                    if (*flag == 1)
                        *flag = 2;
                }
            }
        }
        D_80190B88 = func_8006E498(D_800B0E64, 0xC5465704U);
        func_800C6D5C(D_80190B88, 0, 0);
        D_80190B84 = func_8006E498(D_800B0E64, 0xC5865704U);
        func_800C6D5C(D_80190B84, 0, 0);
        v = func_800D3FD8();
        func_800D3F64(1399, v);
        return (int)func_800CE560((void *)D_800F33E0[2], 20, 14, func_8018F004);
    }

    if (mode == 1) {
        phase = q[12];
        if (phase == 1) {
            q[13]++;
            if (q[13] < 8)
                return 0;
            q[12] = 2;
            q[13] = 0;
            return 0;
        }
        if (phase == 2) {
            q[13]++;
            return q[13] < 64 ? 0 : 1;
        }
        if (phase != 0)
            return 0;

        q[13]++;
        q[0] += q[4];
        q[1] += q[5];
        q[2] += q[6];
        q[4] = q[4] * 31 / 32;
        q[6] = q[6] * 31 / 32;
        q[5] += 5;
        if (D_800E27EC == 1) {
            for (i = 0; i < 6; i++) {
                p = func_800CE610((void *)D_800F33E0[2]);
                if (p != 0) {
                    p[0] = q[0];
                    p[1] = q[1];
                    p[2] = q[2];
                    r = func_80071A54();
                    p[0] = (unsigned short)p[0] - 64 + (r & 0x7F);
                    r = func_80071A54();
                    p[1] = (unsigned short)p[1] - 64 + (r & 0x7F);
                    r = func_80071A54();
                    v = (unsigned short)p[2];
                    p[9] = 1;
                    p[8] = 0;
                    p[2] = v - 64 + (r & 0x7F);
                }
            }
        }
        if (q[1] < D_800942EC - 514)
            return 0;

        angle = func_80071A54();
        for (i = 0; i < 6; i++) {
            p = func_800CE610((void *)D_800F33E0[2]);
            if (p != 0) {
                p[0] = q[0];
                p[1] = q[1];
                p[2] = q[2];
                speed = (func_80071A54() & 0x1F) + 16;
                v = func_80077DC4(angle) * speed;
                p[4] = v / 4096;
                v = func_80077CF4(angle) * speed;
                p[6] = v / 4096;
                p[5] = -8 - (func_80071A54() & 0xF);
                p[9] = 2;
                p[8] = 0;
            }
            angle = (unsigned int)angle + 682U;
        }

        step = extra[2];
        count = extra[3];
        spread = (unsigned int)q[9] - ((step * count) >> 1);
        k = 0;
        while (k < count) {
            p = func_800CE610((void *)D_800F33E0[2]);
            if (p != 0) {
                p[0] = q[0];
                p[1] = q[1];
                p[2] = q[2];
                p[4] = q[8];
                p[5] = q[9];
                v = (unsigned short)p[4];
                r = (unsigned short)q[10];
                p[5] = spread;
                p[9] = 0;
                p[8] = 0;
                p[4] = v - 200;
                p[6] = r;
            }
            k++;
            step = extra[2];
            count = extra[3];
            spread = (unsigned int)spread + step;
        }
        q[12] = 1;
        q[13] = 0;
        return 0;
    }

    if (mode != 2)
        return 0;
    D_800F3374 = 4;
    D_800F3372 = 0;
    size = extra[0] / 20U;
    phase = q[12];
    if (phase == 1) {
        row = D_800E120A;
        v = D_800E11EA;
        D_800F3368 = 32;
        D_800F336A = 2;
        D_800F3376 = 32;
        D_800F3378 = 32;
        v = D_800E2850[v];
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3370 = v;
        clut = func_80077AA4(128, row) & 0xFFFF;
        tex = D_800F336A * q[13] + 32;
        func_800CEE20(q, 0, size * 20, size * 20, tex, clut, phase, 128, 0);
        return 0;
    }
    if (phase != 0)
        return 0;

    angles.x = q[8];
    angles.y = q[9];
    angles.z = q[10];
    angles.z += q[13] * 128;
    D_800F3370 = D_800E2850[D_800E11EA];
    D_800F336C = 3;
    D_800F336E = 0;
    v = func_80077A64(0, 255, 0, 0);
    page = (D_800E2850[D_800E11EA] | v) & 0xFFFF;
    r = D_800F336C;
    row = D_800E1204[r];
    if (r == 4 && D_800F3428 != 0)
        row += 4;
    clut = func_80077AA4(16, row) & 0xFFFF;
    func_800C6EC0(page, clut);
    func_800C6ED8(0);
    func_80079754(&angles, &matrix);
    matrix.t[0] = q[0];
    matrix.t[1] = q[1];
    matrix.t[2] = q[2];
    scale.x = size;
    scale.y = size;
    scale.z = size;
    func_80078CC4(&matrix, &scale);
    func_800C6EF8(D_80190B88);
    func_800C6FA0(D_80190B88, 128);
    func_800C71E4(D_80190B88, &matrix);
    func_800C6F4C(D_80190B88);

    row = D_800E1208;
    D_800F3368 = 16;
    v = D_800E11E8;
    D_800F336A = 1;
    D_800F3376 = 16;
    D_800F3378 = 16;
    v = D_800E2850[v];
    D_800F336C = 2;
    D_800F336E = 0;
    angles.x = 1024;
    angles.y = 0;
    angles.z = 0;
    angles.pad = 1;
    D_800F3370 = v;
    position.x = q[0];
    position.y = q[1];
    position.z = q[2];
    position.y = D_800942EC;
    clut = func_80077AA4(128, row) & 0xFFFF;
    tex = D_800F336A + 253;
    func_800CEE20(&position, &angles, size * 20, size * 20, tex, clut, 2, 128, 0);
    return 0;
}
