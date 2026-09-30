/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m404/RoomEffect_M404ParticleController.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_M404ParticleController` renamed to `func_80193368`, and vendor symbol
 * names mapped to this repo's address names from the vendor sym tables. */
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef float f32;
typedef struct { s32 word[2]; } __attribute__((packed)) RoomTemplate8;
typedef struct RoomM404Node {
    u8 reserved[0x18];
    u8 *state;
} RoomM404Node;
typedef struct RoomM404Channel {
    s32 reserved[2];
    char *pool;
} RoomM404Channel;
typedef struct RoomM404EventState {
    u8 reserved[0xD];
    u8 active;
} RoomM404EventState;
typedef struct RoomM404Particle {
    s16 x, y, z, unused06;
    s16 vx, vy, vz, unused0E;
    s16 state, timer;
} RoomM404Particle;
extern RoomTemplate8 D_8018F208;
extern s32 D_800E27EC;
extern volatile u16 D_800E11EA;
extern u16 D_800E2850[];
extern volatile s16 D_800F3368, D_800F336A, D_800F3376, D_800F3378;
extern volatile s16 D_800F336C, D_800F336E, D_800F3372, D_800F3374, D_800F3370;
extern RoomM404Channel *D_800F32D0, *D_800F33E0;
extern RoomM404EventState *D_800E2368;
extern int func_800D3FD8(void);
extern void func_800D3F64(int, int);
extern int func_800CE560(void *, int, int, void *);
extern void func_800CE8F0(void *, int, void *, void *);
extern void func_800CE9D4(void *, int, void *);
extern RoomM404Particle *func_800CE610(void *);
extern void func_800CFB7C(void *, int, void *);
extern void func_80192540(void);
int func_80193368(int mode, void *unused, char *state) {
    RoomTemplate8 template = D_8018F208;
    s16 position[4];
    s16 target[4];
    char *pool;
    RoomM404Particle *child;
    if (mode == 1)
        goto update;
    if (mode < 2) {
        if (mode == 0)
            goto init;
        goto done;
    }
    if (mode == 2)
        goto configure;
    goto done;
init:
    {
        int handle = func_800D3FD8();
        func_800D3F64(0x5b9, handle);
        if (D_800E2368->active) {
            RoomM404Node **slot = (RoomM404Node **)D_800F32D0->pool;
            if (slot && *slot) {
                u8 *flag = (*slot)->state;
                if (*flag == 1) *flag = 2;
            }
        }
        pool = D_800F33E0->pool;
        return func_800CE560(pool, 20, 55, func_80192540);
    }
update:
    {
        pool = D_800F32D0->pool;
        func_800CE8F0(pool, 3, &template, position);
        pool = D_800F32D0->pool;
        func_800CE9D4(pool, 0, target);
        if (D_800E27EC == 1) {
            pool = D_800F33E0->pool;
            child = func_800CE610(pool);
            if (child) {
                child->x = position[0];
                child->y = position[1];
                child->z = position[2];
                func_800CFB7C(target, *(s16 *)state, &child->vx);
                child->state = 0;
                child->timer = 0;
            }
        }
        if (D_800E27EC < 2) goto done;
        return 2;
    }
configure:
    {
        int idx = D_800E11EA;
        int palette;
        D_800F3368 = 32;
        D_800F336A = 2;
        D_800F3376 = 32;
        D_800F3378 = 32;
        palette = D_800E2850[idx];
        asm volatile("" : : : "memory") ;
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 8;
        D_800F3370 = palette;
    }
done:
    return 0;
}
