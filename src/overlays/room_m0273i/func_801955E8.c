/* room_m0273i — func_801955E8, blob offset 0x6600, 0x39C bytes.
 * Emitter driver for the func_80194E6C particles: mode 0 seeds the timers
 * and allocates the pool, mode 1 spawns a particle pair every 6 frames while
 * the owner is in state 11 and its timer is in [6, 0x23), mode 2 resets the
 * D_800F3368 draw block.
 * Profile: default -O2 -G0 (era_o2_g0), no maspsx knobs. LINK_EXACT.
 * Levers: D_800F33E0 as a struct pointer (in-struct pool load may move above
 * the D_8019AE78 store); case 2 breaks to the shared return 0; the state
 * pointer is checked in t then copied (retail move $s2,$a1); the v[i] read is
 * a temp taken between the +0xA and +0xC stores (it pins the store order);
 * short a in the default arm (an int leaves a move $a1,$v1). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define SH(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

typedef struct {
    short x, y, z, w;
} V4;

typedef struct {
    unsigned short a68;
    unsigned short a6A;
    unsigned short a6C;
    unsigned short a6E;
    unsigned short a70;
    unsigned short a72;
    unsigned short a74;
    unsigned short a76;
    unsigned short a78;
} DrawBlock;

extern DrawBlock D_800F3368;
extern unsigned char *D_800F32D0;
typedef struct {
    unsigned char unknown[8];
    void *pool;
} Context;

extern Context *D_800F33E0;
extern short D_8019AE6C;
extern short D_8019AE70;
extern short D_8019AE74;
extern short D_8019AE78;
extern unsigned char D_8019AEF8;
extern unsigned char D_8019AEFA;
extern short D_8019ACA8;
extern V4 D_8019AE9C[];
extern int func_80052B2C();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_8006DCE4();
extern int func_80194E6C();

int func_801955E8(int mode)
{
    short v[2];
    unsigned char *t;
    unsigned char *s;
    short sp;
    short i;

    switch (mode) {
    case 0:
        D_8019AE6C = 0;
        D_8019AE70 = 0;
        D_8019AE74 = (func_80052B2C() & 7) + 4;
        D_8019AE78 = (func_80052B2C() & 7) + 4;
        return func_800CE560(D_800F33E0->pool, 100, 10, func_80194E6C);
    case 1:
        if (D_8019AEF8 != 0) {
            return 2;
        }
        t = P(D_800F32D0, 8);
        if (B(t, 0xE) != 11) {
            return 0;
        }
        s = t;
        {
            unsigned short raw = SH(s, 0x1A);
            short old;

            if (H(s, 0x16) < 6) {
                return 0;
            }
            if ((short)raw >= 0x23) {
                return 0;
            }
            old = D_8019AE70;
            D_8019AE70 = old - 1;
            if (old > 0) {
                return 0;
            }
        }
        switch (D_8019AEFA) {
        case 0: {
            short c = D_8019AE6C;
            short t = ((c >> 2) << 5) - 0x300;

            v[1] = t;
            v[0] = t;
            D_8019ACA8 = c * 2 + 0x80;
            sp = (c & 3) << 6;
            goto spawn;
        }
        case 1: {
            int a = 0x20 - (func_80052B2C() & 0x3F);

            if (D_8019AE6C & 1) {
                v[0] = a - 0x340;
                v[1] = a - 0x2A0;
            } else {
                v[1] = a - 0x2A0;
                v[0] = a - 0x340;
            }
            sp = 0x10;
            break;
        }
        default: {
            short a = (D_8019AE6C & 3) * 24;

            if (D_8019AE6C & 1) {
                v[0] = a - 0x3C0;
                v[1] = a - 0x2A0;
            } else {
                v[1] = a - 0x2A0;
                v[0] = a - 0x3C0;
            }
            sp = 0x20;
            break;
        }
        }
        D_8019ACA8 = 0x80;
    spawn:
        i = 0;
        do {
            short *q = func_800CE610(D_800F33E0->pool);
            short w;

            if (q == 0) {
                break;
            }
            q[0] = D_8019AE9C[i].x;
            q[1] = D_8019AE9C[i].y;
            q[2] = D_8019AE9C[i].z;
            q[5] = SH(s, 0x3A) + sp * 4;
            w = v[i];
            q[6] = sp;
            sp = -sp;
            q[7] = 0;
            q[0x30] = 0;
            q[0x31] = 0;
            q[4] = w;
            i++;
        } while (i < 2);
        D_8019AE70 = 6;
        D_8019AE6C++;
        {
            void *o = P(s, 0x238);

            func_8006DCE4(0x5D0, P(P(s, 0), 8), H(o, 0x54), H(o, 0x58), H(o, 0x5C));
        }
        return 0;
    case 2:
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0;
        D_800F3368.a76 = D_800F3368.a68;
        D_800F3368.a78 = D_800F3368.a68;
        break;
    }
    return 0;

    return 0;
}
