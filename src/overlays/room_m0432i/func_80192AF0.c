/* room_m0432i — func_80192AF0, blob offset 0x3B08, 0x6C8 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * Effect callback (dispatched from func_800D413C): arm (reset, attach to the
 * actor, start the particle pool with task func_80192548), per-frame (sound
 * 0x5D4/0x5D5 on frame 2, three particle bursts while frame < 3, done at
 * frame 28 or when the actor's +0x16 is clear) and draw (camera RT, sprite
 * plus two rings while frame < 21, then publish the position and palette).
 * Matching notes: the PsyQ RT macro with the matrix_slot barrier (as in
 * m0159i F3A4); one variable for both cosines; `w = s / 2` assigned inside
 * the call so it follows func_80077AA4; the palette index (pre-scaled) and
 * the 0x20 constant read before the "r"(p) barrier, and the table load after
 * a second barrier, keep retail's store order. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;

#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20(%0);" "lw $13, 24(%0);" "ctc2 $12, $5;"                 \
    "lw $14, 28(%0);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")

extern CVECTOR D_8018F1D0;
extern CVECTOR D_8018F1D4;
extern int D_8009D254;
extern unsigned char *D_800E2368;
extern unsigned char *D_800F33E0;
extern int D_800E27EC;
extern int *D_800BCFA4;
extern unsigned short D_800E11EA;
extern unsigned short D_800E11FA;
extern unsigned short D_800E1204[];
extern unsigned short D_800E2850[];
extern short D_800F3368;
extern short D_800F336A;
extern unsigned short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern int D_800F3428;
extern SVECTOR D_801994EC;
extern int func_80071A54();
extern int func_80077AA4();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CE870();
extern void func_800CEE20();
extern void func_800D004C();
extern void func_800D0728();
extern void func_800D3F64();
extern int func_800D3FD8(void);
extern int func_80192548();

int func_80192AF0(int mode, short *p)
{
    SVECTOR ang;
    CVECTOR c1 = D_8018F1D0;
    CVECTOR c2 = D_8018F1D4;
    short *q;
    int i;
    int s;
    int c;
    int w;

    int clut;
    int value;
    int idx;
    int k;
    int **matrix_slot;

    switch (mode) {
    case 0:
        p[5] = 0;
        p[4] = 0;
        p[6] = 1;
        func_800CE870(D_8009D254, 0, p);
        return func_800CE560(*(int *)(D_800F33E0 + 8), 0x14, 0x18, func_80192548);
    case 1:
        if (*(short *)(D_800E2368 + 0x16) == 0) {
            return 1;
        }
        if (D_800E27EC == 2) {
            func_800D3F64(0x5D4, func_800D3FD8());
            func_800D3F64(0x5D5, 0x80);
        }
        if (D_800E27EC < 3) {
            for (i = 0; i < 2; i++) {
                q = func_800CE610(*(int *)(D_800F33E0 + 8));
                if (q != 0) {
                    q[0] = p[0];
                    q[1] = p[1];
                    q[2] = p[2];
                    q[4] = func_80071A54() % 60 - 30;
                    q[5] = func_80071A54() % 60 - 30;
                    q[6] = func_80071A54() % 60 - 30;
                    q[8] = 1;
                    q[9] = 0;
                }
            }
            if (p[6] != 0) {
                for (i = 0; i < 5; i++) {
                    q = func_800CE610(*(int *)(D_800F33E0 + 8));
                    if (q != 0) {
                        q[0] = p[0];
                        q[1] = p[1];
                        q[2] = p[2];
                        q[4] = func_80071A54();
                        q[5] = func_80071A54();
                        q[6] = func_80071A54();
                        q[3] = (func_80071A54() & 0x3FF) + 0x200;
                        q[8] = 0;
                        q[9] = 0;
                    }
                }
            }
            for (i = 0; i < 5; i++) {
                q = func_800CE610(*(int *)(D_800F33E0 + 8));
                if (q != 0) {
                    q[0] = p[0];
                    q[1] = p[1];
                    q[2] = p[2];
                    q[4] = func_80071A54();
                    q[5] = func_80071A54();
                    q[6] = func_80071A54();
                    q[3] = (func_80071A54() & 0x7FF) + 0x400;
                    q[8] = 2;
                    q[9] = 0;
                }
            }
        }
        if (D_800E27EC >= 0x1C) {
            return 1;
        }
        break;
    case 2:
        matrix_slot = &D_800BCFA4;
        asm volatile("" : "=r"(matrix_slot) : "0"(matrix_slot));
        gte_SetRotMatrix(*matrix_slot);
        asm volatile("" : : "r"(matrix_slot));
        D_800F3368 = 0x40;
        D_800F336A = 4;
        D_800F3376 = 0x40;
        D_800F3378 = 0x40;
        value = D_800E2850[D_800E11FA];
        asm volatile("" : : : "memory");
        D_800F336C = 3;
        D_800F336E = 1;
        D_800F3372 = 0;
        D_800F3374 = 0x20;
        D_800F3370 = value;
        if (D_800E27EC < 0x15) {
            s = func_80077DC4((D_800E27EC << 10) / 20) / 32;
            c = func_80077CF4((D_800E27EC << 10) / 20) + 0x1000;
            clut = D_800E1204[D_800F336C] + ((D_800F336C == 4 && D_800F3428) ? 7 : 3);
            func_800CEE20(p, 0, c, c, 4, func_80077AA4(0, clut) & 0xFFFF, 1, w = s / 2, 0);
            func_800D004C(p, 0x320, 0x320, 0xC, 0, 0x1000, 0x1000, &c2, 0, s, 1);
            ang.vy = 0;
            ang.vz = 0;
            ang.vx = 0x400;
            ang.pad = 1;
            c = func_80077CF4((D_800E27EC << 10) / 20);
            func_800D0728(p, 0x44C, 0x578, 0x14, &ang, c, c, 0, &c1, w, 1);
        }
        idx = D_800E11EA * 2;
        k = 0x20;
        D_801994EC.vx = p[0];
        D_801994EC.vy = p[1];
        asm volatile("" : : "r"(p) : "memory");
        D_800F3368 = k;
        D_800F336A = 2;
        D_800F3376 = k;
        D_800F3378 = k;
        D_801994EC.vz = p[2];
        value = *(unsigned short *)((char *)D_800E2850 + idx);
        asm volatile("" : : : "memory");
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 8;
        D_800F3370 = value;
        break;
    }
    return 0;
}
