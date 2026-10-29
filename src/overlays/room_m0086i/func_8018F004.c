/* room_m0086i (PE.IMG room m0086i chunk 2, VRAM 0x8018EFE8)
 * func_8018F004 - blob offset 0x1C, 0x730 bytes. Profile era_o2_g0 (default).
 * LINK_EXACT (era_link_check, -O2 -G0). inline GTE/asm: none; inline asm: one
 * zero-code register barrier (`__asm__("" : "=r"(sp) : "0"(sp))`, keeps
 * &D_800F3368 in $s0 in cmd 2 state 1).
 * Particle callback, a1[9] = state, a1[8] = timer, a2[0] >> 3 = size:
 *   cmd 1 state 0: timer++, func_800CFB7C step from a1+4 by a2's short +4,
 *     add the step to the position; once y >= D_800942EC switch to state 1;
 *     if func_800C6B90(a1, size*41/512) hits and y >= D_800942EC - 0x202,
 *     set the owner's state bits (same block as room_m0023i func_8018FC14)
 *     and switch to state 1. state 1: timer++, y -= 6, done (1) at 8 ticks.
 *     state 2: damped bounce (vx/vz *= 31/32, vy += 4, vy *= -1 at the
 *     floor), done (1) at 32 ticks.
 *   cmd 2 state 0: colour/light setup, rotation matrix from the velocity
 *     angles, mesh D_80190B84 drawn at the position, then a func_800CEE20
 *     sprite at floor height. state 1: func_800CEE20 sprite, brightness
 *     a6A * timer + 200. state 2: func_800CEE20 depth sprite.
 * Levers:
 *   - the empty `do { } while (0);` at the top of cmd 2 state 2 is
 *     load-bearing: its NOTE_INSN_LOOP_END ends cse1's path there (cse2 runs
 *     with after_loop and walks past it), so cse1 leaves state 2's literal 1
 *     argument alone. That keeps the dispatch-constant register short-lived
 *     at cse2 time, so cse2 keeps $a0 for the cmd-1 `== 1` compare and the
 *     state variable `st` ($s2) as state 1's argument. Without it the
 *     callee-saved allocation rotates (s0/s2/s3) and 20 words mismatch;
 *   - D_800942EC and D_800E11E8 are read as arrays (in-struct for gcc 2.7's
 *     alias rule): D_800E11E8[0] must stay after the sh through $s0 in
 *     state 1 while the scalar D_800E1208 load is hoisted above it;
 *   - b.z = a1[8] * 256 gives retail's lh (the << 8 spelling narrows to lhu);
 *   - the D_800E2850[*ip] read precedes the D_800F336C read-back after the
 *     func_80077A64 call; ip = &D_800E11EA is a pointer local ($s0). */

typedef struct { short x, y, z, w; } SVEC;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;

struct S {
    short a68, a6A;
    unsigned short a6C;
    short a6E;
    unsigned short a70;
    short a72, a74, a76, a78;
};

#define P(o, x) (*(void **)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

extern struct S D_800F3368;
extern short D_800942EC[];
extern unsigned short D_800E11EA;
extern unsigned short D_800E11E8[];
extern unsigned short D_800E120A;
extern unsigned short D_800E1208;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800E27EC;
extern int D_800F3428;
extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern void **D_8009D254;
extern int D_80190B84;
extern int func_80077DC4();
extern int func_80077AA4();
extern int func_80077A64();
extern int func_800C6B90();
extern void func_800CFB7C();
extern void func_800CEE20();
extern void func_800C6EC0();
extern void func_800C6ED8();
extern void func_80079754();
extern void func_80078CC4();
extern void func_800C6EF8();
extern void func_800C6FA0();
extern void func_800C71E4();
extern void func_800C6F4C();

int func_8018F004(int a0, short *a1, int *a2)
{
    int s3 = (unsigned int)a2[0] >> 3;
    int st;
    int n;
    unsigned short u;
    int col;
    void *o;
    short *p;
    unsigned short *ip;
    struct S *sp;
    SVEC b;
    SVEC c;
    MATRIX m;
    VECTOR sc;

    switch (a0) {
    case 1:
        switch (a1[9]) {
        case 0:
            a1[8]++;
            func_800CFB7C(a1 + 4, ((short *)a2)[2], &c);
            a1[0] += c.x;
            a1[1] += c.y;
            a1[2] += c.z;
            p = D_800942EC;
            if (a1[1] >= *p) {
                a1[9] = 1;
                a1[8] = 0;
            }
            if (func_800C6B90(a1, (int)(s3 * 41) / 512) == 0) {
                return 0;
            }
            if (a1[1] < *p - 0x202) {
                return 0;
            }
            if (D_800E2368[0xD] != 0) {
                o = D_800F32D0;
                if ((**(int **)P(o, 8) & 0x3F000000) == 0x01000000) {
                    W(*D_8009D254, 0x4C) |= 0x4000;
                    **(int **)P(o, 8) = (**(int **)P(o, 8) & 0xC0FFFFFF) | 0x11000000;
                    **(int **)P(o, 8) |= 0x80000000;
                }
            }
            a1[9] = 1;
            a1[8] = 0;
            break;
        case 1:
            a1[8]++;
            a1[1] -= 6;
            if (a1[8] < 8) {
                break;
            }
            return 1;
        case 2:
            a1[8]++;
            a1[0] += a1[4];
            a1[1] += a1[5];
            a1[2] += a1[6];
            a1[4] = a1[4] * 31 / 32;
            a1[6] = a1[6] * 31 / 32;
            a1[5] += 4;
            if (a1[1] >= D_800942EC[0]) {
                a1[5] *= -1;
            }
            if (a1[8] < 0x20) {
                break;
            }
            return 1;
        default:
            return 0;
        }
        break;
    case 2:
        st = a1[9];
        switch (st) {
        case 0:
            ip = &D_800E11EA;
            b.x = a1[4];
            b.y = a1[5];
            b.z = a1[6];
            b.x += 0x800;
            b.z = a1[8] * 256;
            D_800F3368.a70 = D_800E2850[*ip];
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 0;
            col = func_80077A64(0, 0xFF, 0, 0);
            col = (D_800E2850[*ip] | col) & 0xFFFF;
            u = D_800F3368.a6C;
            n = D_800E1204[u];
            if (u == 4 && D_800F3428 != 0) {
                n += 4;
            }
            func_800C6EC0(col, func_80077AA4(0x20, n) & 0xFFFF);
            func_800C6ED8(0);
            func_80079754(&b, &m);
            m.t[0] = a1[0];
            m.t[1] = a1[1];
            m.t[2] = a1[2];
            sc.vx = s3;
            sc.vy = s3;
            sc.vz = s3;
            func_80078CC4(&m, &sc);
            func_800C6EF8(D_80190B84);
            func_800C6FA0(D_80190B84, 0x80);
            func_800C71E4(D_80190B84, &m);
            func_800C6F4C(D_80190B84);
            D_800F3368.a68 = 0x10;
            D_800F3368.a6A = 1;
            D_800F3368.a76 = 0x10;
            D_800F3368.a78 = 0x10;
            D_800F3368.a70 = D_800E2850[D_800E11E8[0]];
            D_800F3368.a6C = 2;
            D_800F3368.a6E = 0;
            b.x = 0x400;
            b.y = 0;
            b.z = 0;
            b.w = 1;
            c.x = a1[0];
            c.y = a1[1];
            c.z = a1[2];
            c.y = D_800942EC[0];
            func_800CEE20(&c, &b, s3 * 8, s3 * 8, 0xE8,
                          func_80077AA4(0x50, D_800E1208) & 0xFFFF, 2, 0x80, 0);
            break;
        case 1:
            sp = &D_800F3368;
            __asm__("" : "=r"(sp) : "0"(sp));
            sp->a68 = 0x10;
            D_800F3368.a6A = 1;
            D_800F3368.a76 = 0x10;
            D_800F3368.a78 = 0x10;
            D_800F3368.a70 = D_800E2850[D_800E11E8[0]];
            D_800F3368.a6C = 2;
            D_800F3368.a6E = 0;
            func_800CEE20(a1, 0, s3 * 8, s3 * 8, sp->a6A * a1[8] + 0xC8,
                          func_80077AA4(0x10, D_800E1208) & 0xFFFF, st, 0x80, 0);
            break;
        case 2:
            do {
            } while (0);
            func_80077DC4(a1[8] << 5);
            D_800F3368.a68 = 0x10;
            D_800F3368.a6A = 1;
            D_800F3368.a76 = 0x10;
            D_800F3368.a78 = 0x10;
            b.x = 0;
            b.y = 0;
            b.w = 0;
            b.z = D_800E27EC << 7;
            D_800F3368.a70 = D_800E2850[D_800E11EA];
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 0;
            func_800CEE20(a1, &b, 0x555, 0x555, 0x1A,
                          func_80077AA4(0x10, D_800E120A) & 0xFFFF, 1, 0x80, 0);
            break;
        default:
            return 0;
        }
        break;
    default:
        return 0;
    }
    return 0;
}
