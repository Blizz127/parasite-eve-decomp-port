/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m188/RoomEffect_TimedSixthSpawn.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_TimedSixthSpawn` renamed to `func_801930B4`, and vendor symbol
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
typedef struct RoomTimedVector {
    u16 x, y, z, pad;
} RoomTimedVector;
typedef struct RoomTimedContext {
    u8 pad[8];
    void *pool;
} RoomTimedContext;
typedef struct RoomTimedParticle {
    u16 x, y, z;
    u8 pad06[10];
    u16 active, phase, frame;
} RoomTimedParticle;
extern RoomTimedVector D_8018F1DC;
extern RoomTimedContext *D_800F32D0, *D_800F33E0;
extern u8 *D_800E2368;
extern int D_800E27EC;
extern u16 D_800E11EA, D_800E2850[];
extern u16 D_800F336C, D_800F336E, D_800F3370, D_800F3372, D_800F3374;
extern int func_8019251C(int, RoomTimedParticle *);
extern int func_800CE560(void *, int, int, int (*)(int, RoomTimedParticle *));
extern RoomTimedParticle *func_800CE610(void *);
extern void func_800CE8F0(void *, int, RoomTimedVector *, RoomTimedVector *);
int func_801930B4(int mode, s16 *counter)
{
    RoomTimedVector position = D_8018F1DC;
    RoomTimedVector output;
    RoomTimedParticle *particle;
    switch (mode) {
    case 0: {
        u8 *statusSource = D_800E2368;
        *counter = 6;
        if (statusSource[13] != 0) {
            void *object = D_800F32D0->pool;
            if (object != 0) {
                void *inner = *(void **)object;
                if (inner != 0) {
                    u8 *status = *(u8 **)((u8 *)inner + 0x18);
                    if (*status == 1) *status = 2;
                }
            }
        }
        return func_800CE560(D_800F33E0->pool, 24, 32, func_8019251C);
    }
    case 1:
        func_800CE8F0(D_800F32D0->pool, 6, &position, &output);
        if (D_800E27EC % 6 == 0 && *counter > 0) {
            particle = func_800CE610(D_800F33E0->pool);
            if (particle != 0) {
                particle->x = output.x;
                particle->y = output.y;
                particle->z = output.z;
                particle->active = 0;
                particle->frame = 0;
                particle->phase = 0;
            }
            --*counter;
        }
        if (D_800E27EC < 2) goto ret0;
        return 2;
    case 2: {
        u16 palette = D_800E2850[D_800E11EA];
        asm volatile("" : : "r"(palette) : "memory");
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 8;
        D_800F3370 = palette;
        break;
    }
    }
ret0:
    return 0;
}
