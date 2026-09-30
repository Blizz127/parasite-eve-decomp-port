/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m318/func_80193D88.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `func_80193D88` renamed to `func_80193D88`, and vendor symbol
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
typedef struct RoomM318Orbit {
    u16 x, y, z, pad;
    int angle;
} RoomM318Orbit;
typedef struct RoomM318Node {
    char pad[8];
    void *link;
} RoomM318Node;
extern int D_801995B8, D_800E27EC;
extern char D_801995BC[], D_801995DC[];
extern RoomM318Node *D_800F32D0;
extern u16 D_800F3374;
extern void func_800CE870(void *, int, void *);
extern int func_80077DC4(int), func_80077CF4(int);
extern void func_800CF3AC(void *, void *, int);
extern void func_800D004C(void *, int, int, int, void *, int, int, void *, void *, int, int);
extern void func_800D0728(void *, int, int, int, void *, int, int, int, void *, int, int);
int func_80193D88 (int mode, RoomM318Orbit *orbit) {
    GteShortVector position;
    int color[2];
    int scale, sine, cosine, product;
    register int result asm("$2");
    register int delta asm("$4");
    void *palette;
    product = mode;
    switch (product) {
    case 0:
        orbit->angle = D_801995B8;
        D_801995B8 += 0x555;
        func_800CE870(D_800F32D0->link, 0, orbit);
        orbit->y -= 100;
        sine = func_80077DC4(orbit->angle);
        orbit->x += sine * 300 / 4096;
        cosine = func_80077CF4(orbit->angle);
        product = cosine * 300;
        asm volatile("" : : "r"(product));
        result = 0;
        delta = product / 4096;
        orbit->z += delta;
        return result;
    case 1:
        if (D_800E27EC >= 8) return 1;
        break;
    case 2:
        palette = D_801995BC;
        D_800F3374 = 60;
        asm volatile("" ::: "memory");
        position.x = orbit->x;
        position.y = orbit->y;
        position.z = orbit->z;
        scale = (D_800E27EC << 9) + 2048;
        func_800CF3AC(palette, color, D_800E27EC);
        func_800D004C(&position, 500, 500, 16, 0, scale, scale, color, 0, 128, 1);
        func_800D0728(&position, 410, 500, 20, 0, 4096, 4096, 0, color, 128, 3);
        func_800CF3AC(D_801995DC, color, D_800E27EC);
        func_800D004C(&position, 500, 100, 16, 0, scale, scale, color, 0, 128, 1);
        break;
    }
    return 0;
}
