/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m123/func_80195114.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `func_80195114` renamed to `func_80195114`, and vendor symbol
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
typedef struct RoomM123Burst {
    u16 frame;
    u16 phase;
    u16 x, y, z;
    u16 pad;
} RoomM123Burst;
typedef struct RoomM123Wave {
    u16 x, y, z, pad;
    s16 state;
    u16 frame;
} RoomM123Wave;
typedef struct RoomM123Pool {
    char pad[8];
    void *pool;
} RoomM123Pool;
extern GteShortVector D_8018F1CC;
extern RoomM123Pool *D_800F32D0, *D_800F33E0;
extern int D_800E27EC;
extern volatile u16 D_800E11E8;
extern u16 D_800E2850[];
extern volatile u16 D_800F3368, D_800F336A, D_800F336C, D_800F336E;
extern volatile u16 D_800F3370, D_800F3372, D_800F3374, D_800F3376, D_800F3378;
extern int func_80194F68(int, RoomM123Wave *);
extern void func_800CE8F0(void *, int, void *, void *);
extern int func_800D3FD8(void);
extern void func_800D3F64(int, int);
extern int func_800CE560(void *, int, int, int (*)(int, RoomM123Wave *));
extern RoomM123Wave *func_800CE610(void *);
extern int func_80071A54(void);
int func_80195114 (int mode, RoomM123Burst *burst, int *choice) {
    GteShortVector position = D_8018F1CC;
    RoomM123Wave *child;
    int value;
    switch (mode) {
    case 0:
        value = *choice ? 34 : 40;
        func_800CE8F0(D_800F32D0->pool, value, &position, &burst->x);
        burst->frame = 0;
        burst->phase = 0;
        func_800D3F64(0x581, func_800D3FD8());
        return func_800CE560(D_800F33E0->pool, 12, 24, func_80194F68);
    case 1:
        if (D_800E27EC < 32) {
            child = func_800CE610(D_800F33E0->pool);
            if (child != 0) {
                child->x = burst->x;
                child->y = burst->y;
                child->z = burst->z;
                {
                    int sample = func_80071A54();
                    int previous = child->x;
                    child->x = (previous - 128) + (sample & 255);
                }
                {
                    int sample = func_80071A54();
                    int previous = child->y;
                    child->y = (previous - 128) + (sample & 255);
                }
                {
                    int sample = func_80071A54();
                    int previous = child->z;
                    child->z = (previous - 128) + (sample & 255);
                }
                child->state = 0;
                child->frame = 0;
            }
        }
        if (D_800E27EC >= 8) return 2;
        break;
    case 2: {
        int paletteIndex = D_800E11E8;
        int palette;
        D_800F3368 = 16;
        D_800F336A = 1;
        D_800F3376 = 16;
        D_800F3378 = 16;
        palette = D_800E2850[paletteIndex];
        asm volatile("" ::: "memory");
        D_800F336C = 2;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 0;
        D_800F3370 = palette;
        break;
    }
    }
    return 0;
}
