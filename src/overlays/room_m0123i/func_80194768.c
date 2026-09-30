/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m123/RoomEffect_M123ParticleController.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_M123ParticleController` renamed to `func_80194768`, and vendor symbol
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
typedef struct GteMatrix {
    s16 m[3][3];
    s32 t[3];
} GteMatrix;
typedef struct GteMatrixWords {
    u32 r11_r12;
    u32 r13_r21;
    u32 r22_r23;
    u32 r31_r32;
    u32 r33_pad;
    s32 tx, ty, tz;
} GteMatrixWords;
typedef char pe1_static_assert_gte_matrix_words_size [( sizeof(GteMatrixWords) == sizeof(GteMatrix) ) ? 1 : -1] ;
typedef char pe1_static_assert_gte_matrix_words_last_rotation_offset [( ((u32)&((( GteMatrixWords  *)0)->  r33_pad ))  == 16 ) ? 1 : -1] ;
typedef char pe1_static_assert_gte_matrix_words_translation_offset [( ((u32)&((( GteMatrixWords  *)0)->  tx ))  == ((u32)&((( GteMatrix  *)0)->  t ))  ) ? 1 : -1] ;
typedef union GteMatrixStorage {
    GteMatrix matrix;
    u32 words[8];
} GteMatrixStorage;
typedef char pe1_static_assert_gte_matrix_storage_size [( sizeof(GteMatrixStorage) == 32 ) ? 1 : -1] ;
typedef struct GteShortVector {
    s16 x, y, z, pad;
} GteShortVector;
typedef struct GteVector {
    s32 x, y, z, pad;
} GteVector;
GteMatrix * func_80079754 (GteShortVector *angles, GteMatrix *matrix);
GteShortVector * func_80078C34 (const GteMatrix *matrix,
                            const GteShortVector *v, GteShortVector *out);
GteVector * func_80078934 (const GteMatrix *matrix, const GteVector *v,
                         GteVector *out);
void func_800783E4 (void *first, void *second, int first_scale,
                        int second_scale, void *output);
GteMatrix * func_80078CC4 (GteMatrix *matrix, const GteVector *scale);
int func_80079FB4 (int y, int x);
int func_80078004 (int value);
void func_80078134 (GteVector *vector, GteVector *unit);
typedef char pe1_static_assert_gte_matrix_size [( sizeof(GteMatrix) == 32 ) ? 1 : -1] ;
typedef char pe1_static_assert_gte_matrix_translation_offset [( ((u32)&((( GteMatrix  *)0)->  t ))  == 20 ) ? 1 : -1] ;
typedef char pe1_static_assert_gte_short_vector_size [( sizeof(GteShortVector) == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_gte_vector_size [( sizeof(GteVector) == 16 ) ? 1 : -1] ;
typedef struct GteRotation {
    s16 x;
    s16 y;
    s16 z;
    s16 flags;
} GteRotation;
typedef struct RoomM123Particle {
    u16 frame;
    u16 offset;
    s16 scale;
} RoomM123Particle;
typedef struct RoomM123Context {
    u8 pad[8];
    void *pool;
} RoomM123Context;
extern GteShortVector D_8018F1E0;
extern RoomM123Context *D_800F32D0, *D_800F33E0;
extern int D_800E27EC;
extern s16 D_801956A0, D_801956A4;
extern u16 D_801956B0, D_801956B2, D_801956B4;
extern u8 D_801956A8[];
extern u16 D_800E11EA, D_800E2850[];
extern u16 D_800F3368, D_800F336A, D_800F336C, D_800F336E;
extern u16 D_800F3370, D_800F3372, D_800F3374, D_800F3376, D_800F3378;
extern int func_801945EC(int, RoomM123Particle *);
extern int func_800CE560(void *, int, int, int (*)(int, RoomM123Particle *));
extern RoomM123Particle *func_800CE610(void *);
extern void func_800CE8F0(void *, int, GteShortVector *, GteShortVector *);
extern int func_80071A54(void);
extern int func_80077CF4(int);
extern void func_800CE9D4(void *, int, void *);
int func_80194768(int mode)
{
    GteShortVector seed = D_8018F1E0;
    GteShortVector output;
    RoomM123Particle *particle;
    int i;
    int time;
    int scale;
    u16 outX, outY, outZ;
    RoomM123Context *context;
    void *texture;
    u16 palette;
    switch (mode) {
    case 0:
        return func_800CE560(D_800F33E0->pool, 8, 24, func_801945EC);
    case 1:
        if (D_800E27EC == 1) {
            for (i = 0; i < 24; i++) {
                particle = func_800CE610(D_800F33E0->pool);
                if (particle != 0) {
                    particle->frame = (i << 6) / 24;
                    particle->scale = (func_80071A54() & 63) + 64;
                    particle->offset = func_80071A54();
                }
            }
        } else {
            particle = func_800CE610(D_800F33E0->pool);
            if (particle != 0) {
                particle->frame = 0;
                particle->scale = (func_80071A54() & 63) + 64;
                particle->offset = func_80071A54();
            }
        }
        time = D_800E27EC;
        D_801956A4 = 0x1000;
        if (time >= 50)
            D_801956A4 = 0x1000 - ((time - 50) << 12) / 30;
        if (time < 80) goto ret0;
        return 1;
    case 2:
        func_800CE8F0(D_800F32D0->pool, 23, &seed, &output);
        scale = func_80077CF4((D_800E27EC << 11) / 80);
        if (scale < 0)
            scale += 31;
        outX = output.x;
        outY = output.y;
        outZ = output.z;
        context = D_800F32D0;
        asm volatile("" : : "r"(context) : "memory");
        texture = D_801956A8;
        D_801956B0 = outX;
        D_801956B2 = outY;
        D_801956B4 = outZ;
        asm volatile("" : : : "memory");
        D_801956A0 = scale >> 5;
        func_800CE9D4(context->pool, 0, texture);
        D_800F3368 = 16;
        D_800F336A = 1;
        D_800F3376 = 16;
        D_800F3378 = 16;
        palette = D_800E2850[D_800E11EA];
        asm volatile("" : : "r"(palette) : "memory");
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 12;
        D_800F3370 = palette;
        break;
    }
ret0:
    return 0;
}
