/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/scene_e22/func_801981B0.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `func_801981B0` renamed to `func_801981B0`, and vendor symbol
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
typedef struct SceneE22Effect {
    u16 x, y, z;
    s16 speed;
    s16 state;
    s16 frame;
} SceneE22Effect;
extern GteShortVector D_8018F1F4;
extern int D_800E27EC, D_800F3428;
extern u16 D_800F336C, D_800E1204[];
extern int func_80077DC4(int);
extern u16 func_80077AA4 (int, int);
extern void func_800CEE20(void *, void *, int, int, int, int, int, int, void *);
int func_801981B0 (int mode, SceneE22Effect *effect) {
    GteShortVector position = D_8018F1F4;
    int angle, sine, brightness, palette, kind;
    u16 clut;
    switch (mode) {
    case 1: {
        if (effect->state != 0) return 0;
        effect->frame++;
        effect->y -= (effect->speed * 24) / 4096;
        if (effect->frame >= 32) return 1;
        break;
    }
    case 2: {
        if (effect->state != 0) return 0;
        angle = effect->frame << 5;
        position.z = D_800E27EC << 5;
        sine = func_80077DC4(angle);
        brightness = (effect->speed * sine) / 4096;
        kind = D_800F336C;
        palette = D_800E1204[kind];
        if (kind == 4 && D_800F3428) palette += 4;
        clut = func_80077AA4 (112, palette);
        func_800CEE20(effect, &position, brightness, brightness, 0x22, clut, 255, 128, 0);
        break;
    }
    }
    return 0;
}
