/* room_m0086i (PE.IMG room m0086i chunk 2, VRAM 0x8018EFE8)
 * func_801900CC - blob offset 0x10E4, 0x4A8 bytes. Profile era_o2_g0 (default).
 * LINK_EXACT (era_link_check, -O2 -G0). inline GTE/asm: none; inline asm: two
 * zero-code register barriers (`__asm__("" : "=r"(x) : "0"(x))`) on the
 * &D_800F3368 base (keeps it in $s0) and on the 0x10 constant (schedules
 * `li v1,16` first in the state-1 block so reorg steals it into the cmd-2
 * dispatch `beq` delay slot).
 * Particle callback registered by func_80190574 (func_800CE560):
 *   cmd 1 integrates the particle by state a1[9]:
 *     0: timer++, x+=vx, y+=vy, z+=vz, vy++; done (1) at 24 ticks;
 *     1: timer++, position += velocity, vx/vz *= 31/32, vy += 4, bounce
 *        (vy *= -1) once y >= D_800942EC; done (1) at 32 ticks.
 *   cmd 2 seeds the D_800F3368 draw block and submits through func_800CEE20
 *     (state 0: two size/brightness variants keyed on a1[3]; state 1: offset
 *     vector buf, depth D_800E27EC << 7).
 * The outer `default: return 0;` is written first: it keeps state 0's two
 * func_800CEE20 arms from being cross-jumped into each other. */

struct S {
    short a68, a6A;
    unsigned short a6C;
    short a6E;
    unsigned short a70;
    short a72, a74, a76, a78;
};

extern struct S D_800F3368;
extern short D_800942EC[];
extern unsigned short D_800E11EA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800E27EC;
extern int D_800F3428;
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CEE20();

int func_801900CC(int a0, short *a1, int *a2)
{
    int s2 = *a2;
    int s0;
    int n;
    unsigned short u;
    struct S *sp;
    short buf[4];
    int k;

    switch (a0) {
    default:
        return 0;
    case 1:
        switch (a1[9]) {
        case 0:
            a1[8]++;
            a1[0] += a1[4];
            a1[1] += a1[5];
            a1[2] += a1[6];
            a1[5]++;
            if (a1[8] < 0x18) {
                break;
            }
            return 1;
        case 1:
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
        switch (a1[9]) {
        case 0:
            D_800F3368.a68 = 0x20;
            D_800F3368.a6A = 2;
            D_800F3368.a76 = 0x20;
            D_800F3368.a78 = 0x20;
            D_800F3368.a70 = D_800E2850[D_800E11EA];
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 0;
            s0 = func_80077DC4((a1[8] << 10) / 24) / 32;
            if (a1[3] != 0) {
                u = D_800F3368.a6C;
                n = D_800E1204[u];
                if (u == 4 && D_800F3428 != 0) {
                    n += 4;
                }
                func_800CEE20(a1, 0, s2, s2, D_800F3368.a6A * (short)(a1[8] / 6),
                              func_80077AA4(0, n) & 0xFFFF, 2, s0, 0);
            } else {
                u = D_800F3368.a6C;
                n = D_800E1204[u];
                if (u == 4 && D_800F3428 != 0) {
                    n += 4;
                }
                func_800CEE20(a1, 0, s2 * 3 / 2, s2 * 3 / 2, D_800F3368.a6A * ((a1[8] / 2) & 3),
                              func_80077AA4(0, n) & 0xFFFF, 1, s0, 0);
            }
            break;
        case 1:
            k = 0x10;
            __asm__("" : "=r"(k) : "0"(k));
            sp = &D_800F3368;
            __asm__("" : "=r"(sp) : "0"(sp));
            buf[0] = 0;
            buf[1] = 0;
            buf[3] = 0;
            sp->a68 = k;
            D_800F3368.a6A = 1;
            D_800F3368.a76 = k;
            D_800F3368.a78 = k;
            buf[2] = D_800E27EC << 7;
            D_800F3368.a70 = D_800E2850[D_800E11EA];
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 0;
            func_800CEE20(a1, buf, s2 * 3 / 2, s2 * 3 / 2, sp->a6A * a1[3] + 0x1A,
                          func_80077AA4(0x70, D_800E120A) & 0xFFFF, 0xFF, 0x80, 0);
            break;
        }
        break;
    }
    return 0;
}
