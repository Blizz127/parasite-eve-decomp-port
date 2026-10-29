/* room_m0023i (PE.IMG room m0023i chunk 2, VRAM 0x8018EFE8)
 * func_8018F3C8 — blob offset 0x3e0, 0x348 bytes. Profile era_o2_g0 (default).
 * Eve-room particle effect controller (func_800CE560 callback owner of
 * func_8018F004): case 0 resets and registers the particle pool, case 1
 * emits a particle every D_800E2368->0x14 ticks (random or from the owner's
 * position), case 2 sets the draw parameters.
 * Levers: the D_800F3368 draw-parameter block is one struct with the
 * D_800E2850[] table store fifth (after a78), as in room_m0231i
 * func_80192940; the non-random branch stores x, y, z, flag.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only). */
typedef struct { unsigned char bytes[8]; } RoomFxSeed8;
typedef struct { short x, y, z, flag, vx, vy, vz; } Particle;
typedef struct { short pad[4]; short h8, hA; unsigned short count; short mode; } Eff;

extern RoomFxSeed8 D_8018EFF4;
extern char *D_800E2368;
extern int *D_800F32D0;
extern int *D_800F33E0;
extern unsigned short D_800E2850[];
extern unsigned short D_800E11EA;
extern struct { short a68, a6A, a6C, a6E, a70, a72, a74, a76, a78; } D_800F3368;
extern char D_80190758[];
extern int func_80071A54();
extern int func_800CE560();
extern Particle *func_800CE610();
extern void func_800CE870();
extern void func_800CE8F0();
extern int func_8018F004();

int func_8018F3C8(int cmd, Eff *e)
{
    RoomFxSeed8 seed = D_8018EFF4;
    short buf[4];
    Particle *p;
    short lim;
    int idx;

    switch (cmd) {
    case 0:
        e->h8 = 0;
        e->hA = 0;
        e->count = 0;
        switch (*(short *)(D_800E2368 + 0x12)) {
        case 0:
            e->mode = 8;
            break;
        case 1:
            e->mode = 0x10;
            break;
        }
        return func_800CE560(D_800F33E0[2], 0x10, 0xC, func_8018F004);
    case 1:
        func_800CE8F0(D_800F32D0[2], e->mode, &seed, buf);
        e->count++;
        lim = *(short *)(D_800E2368 + 0x14);
        if (lim == 0) {
            return 1;
        }
        if (lim < (short)e->count) {
            e->count = 0;
            func_800CE870(D_800F32D0[2], 0, buf);
            p = func_800CE610(D_800F33E0[2]);
            if (p == 0) {
                return 0;
            }
            if (func_80071A54() & 1) {
                p->x = (func_80071A54() & 0x7FF) - 0x400;
                p->y = func_80071A54();
                p->z = 0;
                p->flag = 0;
            } else {
                p->x = buf[0];
                p->y = buf[1];
                p->z = buf[2];
                p->flag = 1;
            }
            p->vx = func_80071A54() % 70 - 0x23;
            p->vy = func_80071A54() % 70 - 0x23;
            p->vz = func_80071A54() % 70 - 0x23;
        }
        break;
    case 2:
        func_800CE8F0(D_800F32D0[2], e->mode, &seed, D_80190758);
        D_800F3368.a68 = 0x20;
        D_800F3368.a6A = 2;
        D_800F3368.a76 = 0x20;
        D_800F3368.a78 = 0x20;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 0xC;
        break;
    default:
        return 0;
    }
    return 0;
}
