/* room_m0349i — func_8018F7C4, blob offset 0x7DC, 0x90C bytes. Profile
 * era_o2_g0_expand_div (-O2 -G0, MASPSX_EXPAND_DIV: retail's break-7 divide
 * checks); LINK_EXACT at the target VMA (2026-10-08, written from the retail asm).
 * 3-mode emitter: arm (copy the emitter position, start task func_8018F010),
 * per-frame (two timed ring bursts from the emitter plus random spray around
 * the actor joint, sound cue at end+16, done after frame 16), and draw setup
 * (two glow sprites at joint 13 with the room palette).
 * Matching notes: the EffectHead member store and D_800942EC[0] keep gcc
 * 2.7's case-0 store order; D_800F336A[0] keeps its address live across the
 * second sprite call; `x + -C` keeps the spray's (e - C) + rand order; the
 * case-2 palette read sits before a barrier. */

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
typedef struct { short x, y, z, pad; int rate; int period; int end; } EmitDef;
typedef struct { short x; } EffectHead;
extern SVECTOR D_8018F000;
extern SVECTOR D_8018F008;
extern CVECTOR D_8018EFFC;
extern unsigned short D_800942EC[];
extern unsigned short D_800E11E8;
extern unsigned short D_800E11EA;
extern unsigned short D_800E1204[];
extern int D_800E27EC;
extern unsigned short D_800E2850[];
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern short D_800F3368;
extern short D_800F336A[];
extern unsigned short D_800F336C;
extern short D_800F336E;
extern unsigned short D_800F3370;
extern short D_800F3372;
extern short D_800F3374;
extern short D_800F3376;
extern short D_800F3378;
extern int D_800F3428;
extern void func_8018F010();
extern int func_80071A54();
extern int func_80077AA4();
extern int func_80077CF4();
extern int func_80077DC4();
extern void func_800866A4();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CE870();
extern void func_800CE8F0();
extern void func_800CEE20();

int func_8018F7C4(int mode, short *p, EmitDef *d)
{
    SVECTOR pos;
    SVECTOR out;
    SVECTOR off0;
    SVECTOR off1;
    CVECTOR col;
    short *e;
    int rate;
    int period;
    int ang;
    int r;
    int sz;
    int clut;
    int pal;

    off0 = D_8018F000;
    off1 = D_8018F008;
    col = D_8018EFFC;
    switch (mode) {
    case 0:
        ((EffectHead *)p)->x = d->x;
        p[1] = d->y;
        p[2] = d->z;
        p[1] = D_800942EC[0];
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x14, 0x46, func_8018F010);
    case 1:
        func_800CE870(*(void **)(D_800F32D0 + 8), 0, &pos);
        rate = d->rate;
        period = d->period;
        if (D_800E27EC <= d->end) {
            if (D_800E27EC % period == 0) {
                e = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (e) {
                    e[0] = p[0];
                    e[1] = p[1];
                    e[2] = p[2];
                    ang = func_80071A54();
                    r = rate * (func_80071A54() & 0xFF) / 256;
                    e[0] += func_80077DC4(ang) * r / 4096;
                    e[1] -= func_80071A54() & 0x1F;
                    e[2] += func_80077CF4(ang) * r / 4096;
                    e[3] = func_80071A54();
                    e[4] = func_80071A54() % 32 - 16;
                    e[5] = -(func_80071A54() % 32);
                    e[6] = func_80071A54() % 32 - 16;
                    e[8] = 0;
                    e[9] = 0;
                }
            }
            if (D_800E27EC % period == period / 2) {
                e = func_800CE610(*(void **)(D_800F33E0 + 8));
                if (e) {
                    e[0] = p[0];
                    e[1] = p[1];
                    e[2] = p[2];
                    ang = func_80071A54();
                    r = rate * (func_80071A54() & 0xFF) / 256;
                    e[0] += func_80077DC4(ang) * r / 4096;
                    e[1] -= func_80071A54() & 0x1F;
                    e[2] += func_80077CF4(ang) * r / 4096;
                    e[3] = func_80071A54();
                    e[4] = func_80071A54() % 24 - 12;
                    e[5] = -(func_80071A54() % 24);
                    e[6] = func_80071A54() % 24 - 12;
                    e[8] = 1;
                    e[9] = 0;
                }
            }
        }
        if (!(D_800E27EC & 1) && !(func_80071A54() & 1)) {
            e = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (e) {
                e[0] = pos.vx;
                e[1] = pos.vy;
                e[2] = pos.vz;
                e[0] += (func_80071A54() & 0x1FF) + -0x100;
                e[1] += func_80071A54() % 800 + -0x190;
                e[2] += (func_80071A54() & 0x1FF) + -0x100;
                e[7] = (func_80071A54() & 0x1FF) + 0x2AA;
                e[3] = func_80071A54();
                e[4] = func_80071A54() % 24 - 12;
                e[5] = -(func_80071A54() % 24);
                e[6] = func_80071A54() % 24 - 12;
                e[8] = 2;
                e[9] = 0;
            }
        }
        if (D_800E27EC == d->end + 0x10 && p[4] != -1) {
            func_800866A4(p[4], 0);
        }
        if (D_800E27EC >= 0x10) {
            return 2;
        }
        break;
    case 2:
        D_800F3368 = 0x10;
        D_800F336A[0] = 1;
        D_800F3376 = 0x10;
        D_800F3378 = 0x10;
        pal = D_800E2850[D_800E11E8];
        asm volatile("" : : : "memory");
        D_800F336C = 2;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 4;
        D_800F3370 = pal;
        sz = 0x2AA;
        ang = (func_80071A54() & 0x1F) + 0x20;
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 0xD, &off0, &out);
        clut = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428) {
            clut += 4;
        }
        func_800CEE20(&out, 0, sz, sz, D_800F336A[0] + 0xD8, func_80077AA4(0x20, clut) & 0xFFFF, 1, ang, &col);
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 0xD, &off1, &out);
        clut = D_800E1204[D_800F336C];
        if (D_800F336C == 4 && D_800F3428) {
            clut += 4;
        }
        func_800CEE20(&out, 0, sz, sz, D_800F336A[0] + 0xD8, func_80077AA4(0x20, clut) & 0xFFFF, 1, ang, &col);
        D_800F3368 = 0x40;
        D_800F336A[0] = 4;
        D_800F3376 = 0x40;
        D_800F3378 = 0x40;
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3370 = D_800E2850[D_800E11EA];
        break;
    }
    return 0;
}
