/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m123/RoomEffect_M123PulseController.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_M123PulseController` renamed to `func_80194C04`, and vendor symbol
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
typedef struct RoomPulseParticle {
    u16 x, y, z, pad;
    s16 state, frame;
} RoomPulseParticle;
typedef struct RoomPulseContext {
    u8 pad[8];
    void *pool;
} RoomPulseContext;
typedef struct RoomPulseObject RoomPulseObject;
typedef struct RoomPulsePool {
    RoomPulseObject *object;
} RoomPulsePool;
struct RoomPulseObject {
    u32 flags;
    u8 pad[0x14];
    u8 *status;
};
typedef struct RoomPulseGlobal {
    u8 pad[0x4C];
    u32 flags;
} RoomPulseGlobal;
extern RoomPulseContext *D_800F32D0, *D_800F33E0;
extern RoomPulseGlobal **D_8009D254;
extern u8 *D_800E2368;
extern int D_800E27EC;
extern u16 D_800942EC, D_800E11EA, D_800E2850[];
extern u16 D_800F3368, D_800F336A, D_800F336C, D_800F336E;
extern u16 D_800F3370, D_800F3372, D_800F3374, D_800F3376, D_800F3378;
extern void *D_800B0E64;
extern int func_80194A70(int, RoomPulseParticle *, int *);
extern void func_800CE870(void *, int, void *);
extern int func_800CE560(void *, int, int, int (*)(int, RoomPulseParticle *, int *));
extern RoomPulseParticle *func_800CE610(void *);
extern int func_800D3FD8(void);
extern void func_8006DF50(void *, int, int, int, int);
int func_80194C04(int mode, RoomPulseParticle *particle)
{
    RoomPulseParticle *next;
    RoomPulsePool *pool;
    RoomPulseObject *object;
    RoomPulseContext *context;
    void **soundSlot;
    void *currentSound;
    int time;
    int volume;
    u16 palette;
    switch (mode) {
    case 0:
        func_800CE870(D_800F32D0->pool, 1, &particle->z);
        particle->x = 0;
        particle->y = 0;
        if (D_800E2368[13]) {
            pool = D_800F32D0->pool;
            if (pool != 0 && pool->object != 0) {
                object = pool->object;
                if (*object->status == 1)
                    *object->status = 2;
            }
        }
        soundSlot = &D_800B0E64;
        currentSound = *soundSlot;
        if (currentSound != 0) {
            volume = 0x7F;
            time = func_800D3FD8();
            func_8006DF50(*soundSlot, 0x582, time, 0x80, volume);
            currentSound = *soundSlot;
            asm("" : : "r"(currentSound) : "memory");
            if (currentSound != 0)
                func_8006DF50(*soundSlot, 0x5B1, 0x80, 0x80, volume);
        }
        return func_800CE560(D_800F33E0->pool, 12, 8, func_80194A70);
    case 1:
        if (D_800E27EC < 32 && D_800E27EC % 6 == 0) {
            next = func_800CE610(D_800F33E0->pool);
            if (next != 0) {
                next->x = particle->z;
                next->y = particle->pad;
                next->z = particle->state;
                asm volatile("" : : : "memory");
                next->state = 0;
                next->frame = 0;
                next->y = D_800942EC;
            }
        }
        if (D_800E27EC == 32 && D_800E2368[13]) {
            context = D_800F32D0;
            if ((((RoomPulsePool *)context->pool)->object->flags & 0x3F000000) == 0x01000000) {
                (*D_8009D254)->flags |= 0x4000;
                ((RoomPulsePool *)context->pool)->object->flags =
                    (((RoomPulsePool *)context->pool)->object->flags & 0xC0FFFFFF) | 0x21000000;
                ((RoomPulsePool *)context->pool)->object->flags |= 0x80000000;
            }
        }
        if (D_800E27EC < 8) goto ret0;
        return 2;
    case 2:
        D_800F3368 = 32;
        D_800F336A = 2;
        D_800F3376 = 32;
        D_800F3378 = 32;
        palette = D_800E2850[D_800E11EA];
        asm volatile("" : : "r"(palette) : "memory");
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0;
        D_800F3370 = palette;
        break;
    }
ret0:
    return 0;
}
