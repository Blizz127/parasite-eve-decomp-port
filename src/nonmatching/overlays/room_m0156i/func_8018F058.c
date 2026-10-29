/* NON_MATCHING: functionally equivalent to retail room_m0156i 0x70..0x1248 (func_8018F058).
 * Mode 1 is byte-exact; mode 2 differs in register allocation and division
 * sequences only. Not linked into the exact rebuild (the YAML span stays asm);
 * native port differential-tested by tools/analysis/e21_diff.py. */
/* room_m0156i - func_8018F058, blob offset 0x70, 0x11D8 bytes. Profile
 * room_m0156i_dispatch_8018F00C_8018F024 (-O2 -G0, MASPSX_DISPATCH_FOLD on
 * the two header jump tables, MASPSX_EXPAND_DIV); written from the retail
 * asm (2026-10-09).
 * Fireball effect for the m0156i roof fight (effect callback mode/p/def).
 * mode 1 (6 kinds): 0 the falling fireball (jitter, trail sparks, hit test
 * r=150 that can flag the actor, floor test -> kind 5; on landing kind 1 and
 * an 8x2 debris burst), 1 the blast (hit test r=400 for 9 frames, 16 frames),
 * 2..4 debris integrators (127/128, 511/512, 61/62 drag), 5 a 6-frame wait.
 * mode 2 draws each kind with func_800CEE20 / func_800D004C / func_800D1DEC.
 * Was the Day 4 m0156i battle stop CPU_BOUNDARY/REFUSED pc=0x8018F058. */

#define gte_SetRotMatrix(r0) __asm__ volatile(                       \
    "lw $12, 0(%0);" "lw $13, 4(%0);" "ctc2 $12, $0;" "ctc2 $13, $1;"   \
    "lw $12, 8(%0);" "lw $13, 12(%0);" "lw $14, 16(%0);"                \
    "ctc2 $12, $2;" "ctc2 $13, $3;" "ctc2 $14, $4;"                     \
    "lw $12, 20(%0);" "lw $13, 24(%0);" "ctc2 $12, $5;"                 \
    "lw $14, 28(%0);" "ctc2 $13, $6;" "ctc2 $14, $7"                    \
    : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
extern SVECTOR D_8018EFF4;
extern SVECTOR D_8018EFFC;
extern CVECTOR D_8018F004;
extern short D_80192BFC;
extern short D_800942EC[];
extern unsigned short D_8009D1CC;
extern int D_8009D248;
extern unsigned char **D_8009D254;
extern int *D_800BCFA4;
extern unsigned short D_800E11EA;
extern unsigned short D_800E11FA;
extern unsigned short D_800E1204[];
extern unsigned short D_800E120A;
extern unsigned char *D_800E2368;
extern int D_800E27EC;
extern unsigned short D_800E2850[];
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern short D_800F3368[];   /* sprite parameter block 0x800F3368..0x800F3379 */
#define SPR_W   D_800F3368[0]
#define SPR_K   D_800F3368[1]
#define SPR_TP  (*(unsigned short *)&D_800F3368[2])
#define SPR_AB  D_800F3368[3]
#define SPR_PAL (*(unsigned short *)&D_800F3368[4])
#define SPR_U   D_800F3368[5]
#define SPR_V   D_800F3368[6]
#define SPR_SW  D_800F3368[7]
#define SPR_SH  D_800F3368[8]
extern int D_800F3428;
extern int func_8001CAB0(int, int, int, int);
extern int func_80071A54();
extern int func_80077AA4(int, int);
extern int func_80077CF4(int);
extern int func_80077DC4(int);
extern int func_800C6B90(void *, int);
extern short *func_800CE610();
extern void func_800CEE20();
extern void func_800D004C();
extern void func_800D1DEC();

int func_8018F058(int mode, short *p)
{
    SVECTOR a;
    SVECTOR b;
    SVECTOR pos;
    CVECTOR c;
    SVECTOR d;
    SVECTOR e;
    short *s;
    int i;
    register int r asm("$18");
    int ang;
    int t;
    int u;
    int clut;
    unsigned char *chan;
    int **matrix_slot;

    a = D_8018EFF4;
    b = D_8018EFFC;
    c = D_8018F004;
    switch (mode) {
    case 1:
        switch (p[8]) {
        case 0:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[0] += (func_80071A54() & 0x1F) - 16;
            p[1] += (func_80071A54() & 0x1F) - 16;
            p[2] += (func_80071A54() & 0x1F) - 16;
            if (p[1] >= D_800942EC[0]) {
                p[8] = 1;
                p[9] = 0;
                for (i = 0; i < 8; i++) {
                    s = func_800CE610(*(void **)(D_800F33E0 + 8));
                    if (s) {
                        s[0] = p[0];
                        s[1] = p[1];
                        s[2] = p[2];
                        r = (func_80071A54() & 0x1F) + 100;
                        ang = D_80192BFC + (func_80071A54() & 0x1FF) - 256;
                        s[4] = func_80077DC4(ang) * r / 4096;
                        s[6] = func_80077CF4(ang) * r / 4096;
                        s[5] = -(func_80071A54() & 0x1F) - 24;
                        s[8] = 4;
                        s[9] = 0;
                    }
                    s = func_800CE610(*(void **)(D_800F33E0 + 8));
                    if (s) {
                        s[0] = p[0];
                        s[1] = p[1];
                        s[2] = p[2];
                        {
                            int m = func_80071A54() & 0xF;
                            asm("" : "=r"(m) : "0"(m));
                            r = m + 48;
                        }
                        ang = D_80192BFC + (func_80071A54() & 0x1FF) - 256;
                        s[4] = func_80077DC4(ang) * r / 4096;
                        s[6] = func_80077CF4(ang) * r / 4096;
                        s[5] = -(func_80071A54() & 0xF);
                        s[8] = 3;
                        s[9] = 0;
                    }
                }
            }
            if (p[9] & 1) {
                s = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (s) {
                    s[0] = p[0];
                    s[1] = p[1];
                    s[2] = p[2];
                    s[0] += (func_80071A54() & 0x7F) - 64;
                    s[1] += (func_80071A54() & 0x7F) - 64;
                    s[2] += (func_80071A54() & 0x7F) - 64;
                    s[4] = (func_80071A54() & 0xF) - 8;
                    s[5] = (func_80071A54() & 0xF) - 8;
                    s[6] = (func_80071A54() & 0xF) - 8;
                    s[8] = 2;
                    s[9] = 0;
                }
            }
            if ((short)(p[9] % 3) == 0) {
                s = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (s) {
                    s[0] = p[0];
                    s[1] = p[1];
                    s[2] = p[2];
                    s[4] = (func_80071A54() & 0x1F) - 16;
                    s[5] = (func_80071A54() & 0x1F) - 16;
                    s[6] = (func_80071A54() & 0x1F) - 16;
                    s[8] = 3;
                    s[9] = 0;
                }
            }
            if (p[9] & 1) {
                s = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (s) {
                    s[0] = p[0];
                    s[1] = p[1];
                    s[2] = p[2];
                    s[4] = (func_80071A54() & 0x7F) - 64;
                    s[5] = (func_80071A54() & 0x7F) - 64;
                    s[6] = (func_80071A54() & 0x7F) - 64;
                    s[8] = 4;
                    s[9] = 0;
                }
            }
            if (func_800C6B90(p, 150)) {
                if (D_800E2368[0xD] != 0) {
                    chan = D_800F32D0;
                    if ((***(int ***)(chan + 8) & 0x3F000000) == 0x01000000) {
                        *(int *)(*D_8009D254 + 0x4C) |= 0x4000;
                        **(int **)*(int ***)(chan + 8) = (**(int **)*(int ***)(chan + 8) & 0xC0FFFFFF) | 0x19000000;
                        **(int **)*(int ***)(chan + 8) |= 0x80000000;
                    }
                }
            }
            if (!func_8001CAB0(p[0] << 16, p[2] << 16, D_8009D248, D_8009D1CC)) {
                p[8] = 5;
                p[9] = 0;
            }
            break;
        case 1:
            p[9]++;
            if (func_800C6B90(p, 400) && p[9] < 9) {
                if (D_800E2368[0xD] != 0) {
                    chan = D_800F32D0;
                    if ((***(int ***)(chan + 8) & 0x3F000000) == 0x01000000) {
                        *(int *)(*D_8009D254 + 0x4C) |= 0x4000;
                        **(int **)*(int ***)(chan + 8) = (**(int **)*(int ***)(chan + 8) & 0xC0FFFFFF) | 0x19000000;
                        **(int **)*(int ***)(chan + 8) |= 0x80000000;
                    }
                }
            }
            if (p[9] >= 16) {
                return 1;
            }
            break;
        case 2:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 127 / 128;
            p[6] = p[6] * 127 / 128;
            p[5] -= 1;
            if (p[1] >= D_800942EC[0]) {
                p[5] *= -1;
            }
            if (p[9] >= 16) {
                return 1;
            }
            break;
        case 3:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 511 / 512;
            p[6] = p[6] * 511 / 512;
            p[5] -= 3;
            if (p[1] >= D_800942EC[0]) {
                p[5] *= -1;
            }
            if (p[9] >= 24) {
                return 1;
            }
            break;
        case 4:
            p[9]++;
            p[0] += p[4];
            p[1] += p[5];
            p[2] += p[6];
            p[4] = p[4] * 61 / 62;
            p[6] = p[6] * 61 / 62;
            p[5] += 4;
            if (p[1] >= D_800942EC[0]) {
                p[5] *= -1;
            }
            if (p[9] >= 16) {
                return 1;
            }
            break;
        case 5:
            p[9]++;
            if (p[9] >= 6) {
                return 1;
            }
            break;
        }
        break;
    case 2:
        matrix_slot = &D_800BCFA4;
        asm volatile("" : "=r"(matrix_slot) : "0"(matrix_slot));
        gte_SetRotMatrix(*matrix_slot);
        asm volatile("" : : "r"(matrix_slot));
        switch (p[8]) {
        case 0:
            SPR_K = 2;
            b.vz = D_800E27EC << 8;
            SPR_W = SPR_SW = SPR_SH = 0x20;
            clut = D_800E1204[SPR_TP];
            if (SPR_TP == 4 && D_800F3428) {
                clut += 4;
            }
            func_800CEE20(p, &b, 0x1800, 0x1800, 160, func_80077AA4(80, clut) & 0xFFFF, 1, 128, 0);
            u = (D_800E27EC & 1) * 32 + 128;
            func_800D004C(p, 200, 200, 8, 0, 0x3000, 0x3000, &c, 0, u, 1);
            e.vx = 0x400;
            e.vy = 0;
            e.vz = 0;
            e.pad = 1;
            d.vx = p[0];
            d.vy = D_800942EC[0];
            d.vz = p[2];
            clut = D_800E1204[SPR_TP];
            if (SPR_TP == 4 && D_800F3428) {
                clut += 4;
            }
            func_800CEE20(&d, &e, 0x3000, 0x3000, 0, func_80077AA4(0, clut) & 0xFFFF, 3, 80, 0);
            break;
        case 1:
            SPR_W = 0x40;
            SPR_K = 4;
            SPR_SW = 0x40;
            SPR_SH = 0x40;
            {
                int sz;
                int al;
                int rr;

                sz = func_80077CF4(p[9] << 6) + 0x1000;
                al = func_80077DC4(p[9] << 6) / 64;
                a.vz = D_80192BFC + 0x400;
                for (i = 0; i < 3; i++) {
                    pos.vx = p[0];
                    pos.vy = p[1];
                    pos.vz = p[2];
                    rr = ((p[9] * i) << 4) + 200;
                    pos.vx += func_80077DC4(D_80192BFC) * rr / 4096;
                    pos.vz += func_80077CF4(D_80192BFC) * rr / 4096;
                    clut = D_800E1204[SPR_TP];
                    if (SPR_TP == 4 && D_800F3428) {
                        clut += 4;
                    }
                    func_800CEE20(&pos, &a, sz, sz, 108, func_80077AA4(32, clut) & 0xFFFF, 1, al, 0);
                }
            }
            break;
        case 2: {
            int sz;

            sz = func_80077CF4((p[9] << 10) / 24) / 2 + 0x1000;
            r = p[9];
            if (r >= 8) {
                r += 8;
            }
            SPR_W = 0x20;
            SPR_K = 2;
            SPR_SW = 0x20;
            SPR_SH = 0x20;
            clut = D_800E1204[SPR_TP];
            if (SPR_TP == 4 && D_800F3428) {
                clut += 4;
            }
            func_800CEE20(p, 0, sz * 9 / 4, sz * 9 / 4, SPR_K * r + 32, func_80077AA4(16, clut) & 0xFFFF, 1, 128, 0);
            break;
        }
        case 3:
            t = func_80077CF4((p[9] << 10) / 24) / 2 + 0x1000;
            u = func_80077DC4(p[9] * 1204 / 24) / 128 + 40;
            SPR_W = 0x20;
            SPR_K = 2;
            SPR_SW = 0x20;
            SPR_SH = 0x20;
            clut = D_800E1204[SPR_TP];
            if (SPR_TP == 4 && D_800F3428) {
                clut += 4;
            }
            func_800CEE20(p, 0, t * 3, t * 3, SPR_K * (short)(p[9] / 6) + 128, func_80077AA4(48, clut) & 0xFFFF, 1, u, 0);
            break;
        case 4:
            t = func_80077DC4(p[9] << 6) / 32 + 64;
            SPR_W = 0x20;
            SPR_K = 2;
            SPR_SW = 0x20;
            SPR_SH = 0x20;
            func_800D1DEC(p, &c, t, 1);
            break;
        case 5:
            SPR_W = 0x20;
            SPR_K = 2;
            SPR_SW = 0x20;
            SPR_SH = 0x20;
            SPR_TP = 3;
            SPR_AB = 1;
            SPR_U = 5;
            SPR_V = 0x20;
            SPR_PAL = D_800E2850[D_800E11FA];
            func_800CEE20(p, 0, 0x2000, 0x2000, SPR_K * (p[9] / 2) + 96, func_80077AA4(0, D_800E120A + 2) & 0xFFFF, 1, 80, 0);
            SPR_TP = 3;
            SPR_AB = 0;
            SPR_U = 0;
            SPR_V = 8;
            SPR_PAL = D_800E2850[D_800E11EA];
            break;
        }
        break;
    }
    return 0;
}
