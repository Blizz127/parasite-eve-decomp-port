/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m156+2/RoomEffect_RandomPairController.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_RandomPairController` renamed to `func_801908B0`, and vendor symbol
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
typedef struct RoomOrbitPairTemplate8 {
    s32 word[2];
} __attribute__((packed)) RoomOrbitPairTemplate8;
typedef struct RoomOrbitPairChannel {
    s32 reserved[2];
    char *pool;
} RoomOrbitPairChannel;
typedef struct RoomOrbitPairNode {
    u8 reserved[0x18];
    u8 *state;
} RoomOrbitPairNode;
typedef struct RoomOrbitPairEventState {
    u8 reserved[0xD];
    u8 active;
} RoomOrbitPairEventState;
typedef struct RoomOrbitPairParticle {
    s16 x, y, z, pad6;
    s16 vx, vy, vz, padE;
    s16 state, timer;
} RoomOrbitPairParticle;
typedef char pe1_static_assert_room_orbit_pair_pool_offset [( ((u32)&((( RoomOrbitPairChannel  *)0)->  pool ))  == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_orbit_pair_node_state_offset [( ((u32)&((( RoomOrbitPairNode  *)0)->  state ))  == 0x18 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_orbit_pair_event_active_offset [( ((u32)&((( RoomOrbitPairEventState  *)0)->  active ))  == 0xD ) ? 1 : -1] ;
typedef char pe1_static_assert_room_orbit_pair_particle_size [( sizeof(RoomOrbitPairParticle) == 0x14 ) ? 1 : -1] ;
extern RoomOrbitPairTemplate8 D_8018F03C;
extern RoomOrbitPairChannel *D_800F32D0, *D_800F33E0;
extern RoomOrbitPairEventState *D_800E2368;
extern s32 D_800E27EC;
extern volatile u16 D_800E11EA;
extern u16 D_800E2850[];
extern s16 D_80192BFC;
extern u16 D_800F336C;
extern volatile s16 D_800F336E, D_800F3372, D_800F3374, D_800F3370;
extern s16 D_800F336A;
extern volatile s16 D_800F3368, D_800F3376, D_800F3378;
extern int func_800CE560(void *, int, int, void *);
extern void func_800CE8F0(void *, int, void *, void *);
extern void func_800CE9D4(void *, int, void *);
extern RoomOrbitPairParticle *func_800CE610(void *);
extern int func_80077DC4 (int);
extern int func_80077CF4 (int);
extern void func_8018F058(void);
int func_8019050C(int mode, RoomOrbitPairParticle *particle);
extern int func_800D3FD8(void);
extern void func_800D3F64(int, int);
extern int func_80071A54(void);
int func_80190230(int mode, void *unused, s32 *state);
int func_801908B0(int mode, void *unused, void *state);
int func_801908B0(int mode, void *unused, void *state) {
    RoomOrbitPairTemplate8 template = D_8018F03C;
    s16 position[4];
    s16 target[4];
    char *pool;
    RoomOrbitPairParticle *child;
    int angle, radius;
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
        func_800D3F64(0x58B, handle);
        pool = D_800F33E0->pool;
        return func_800CE560(pool, 20, 16, func_8019050C);
    }
update:
    pool = D_800F32D0->pool;
    func_800CE8F0(pool, 7, &template, position);
    pool = D_800F32D0->pool;
    func_800CE9D4(pool, 0, target);
    angle = -target[1] + 0x400;
    if (D_800E27EC < 47) {
        if (D_800E27EC & 1) {
            pool = D_800F33E0->pool;
            child = func_800CE610(pool);
            if (child) {
                child->x = position[0];
                child->y = position[1];
                child->z = position[2];
                radius = (func_80071A54() & 15) + 8;
                angle = angle + (func_80071A54() & 0x1FF) - 0x100;
                child->vx = func_80077DC4 (angle) * radius / 4096;
                child->vz = func_80077CF4 (angle) * radius / 4096;
                child->vy = -(func_80071A54() & 7);
                child->state = 0;
                child->timer = 0;
            }
        } else {
            pool = D_800F33E0->pool;
            child = func_800CE610(pool);
            if (child) {
                child->x = position[0];
                child->y = position[1];
                child->z = position[2];
                radius = (func_80071A54() & 7) + 4;
                angle = angle + (func_80071A54() & 0x1FF) - 0x100;
                child->vx = func_80077DC4 (angle) * radius / 4096;
                child->vz = func_80077CF4 (angle) * radius / 4096;
                child->vy = -(func_80071A54() & 15);
                child->state = 1;
                child->timer = 0;
            }
        }
    }
    if (D_800E27EC < 2) goto done;
    return 2;
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
