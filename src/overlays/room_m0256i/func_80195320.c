/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m256/func_80195320.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `func_80195320` renamed to `func_80195320`, and vendor symbol
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
typedef struct FieldAnimBurstParameters {
    u8 r, g, b, reserved03;
    u8 parameter04, parameter05;
    u16 reserved06;
    s16 parameter08, parameter0A;
} FieldAnimBurstParameters;
typedef char pe1_static_assert_field_anim_burst_parameters_size [( sizeof(FieldAnimBurstParameters) == 12 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_burst_parameter04 [( ((u32)&((( FieldAnimBurstParameters  *)0)->  parameter04 ))  == 4 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_burst_parameter08 [( ((u32)&((( FieldAnimBurstParameters  *)0)->  parameter08 ))  == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_burst_parameter0a [( ((u32)&((( FieldAnimBurstParameters  *)0)->  parameter0A ))  == 10 ) ? 1 : -1] ;
extern GteMatrix D_800F3478, D_800F33C0, D_800F32B0;
extern GteVector D_800C220C, D_800C221C, D_800C222C;
extern FieldAnimBurstParameters D_800E2298, D_800E2250, D_800E27E0, D_800F3460;
extern char D_800E0EB8[];
int func_800CCBA8(char *object);
typedef int (*FieldAnimTaskCallback)(int mode, void *state);
typedef struct FieldAnimTaskSlot {
    u16 id;               
    u16 age;
    char *start;
    char *end;
} FieldAnimTaskSlot;
typedef struct FieldAnimTaskTable {
    int (*callbacks[8])(int mode, void *state, int argument);
    u16 sizes[8];
} FieldAnimTaskTable;
typedef struct FieldAnimTaskContext {
    u16 *script;
    char *cursor;
    int argument;
    u8 count;
    u8 flags;
    u16 delay;
    u16 used;
    s16 variables[7];
    FieldAnimTaskSlot slots[8];
    FieldAnimTaskTable *table;
    char arena[0x97C];
} FieldAnimTaskContext;
typedef char pe1_static_assert_field_anim_task_slot_size [( sizeof(FieldAnimTaskSlot) == 0xC ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_task_slot_start [( ((u32)&((( FieldAnimTaskSlot  *)0)->  start ))  == 4 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_task_table_sizes [( ((u32)&((( FieldAnimTaskTable  *)0)->  sizes ))  == 0x20 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_task_context_count [( ((u32)&((( FieldAnimTaskContext  *)0)->  count ))  == 0xC ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_task_context_used [( ((u32)&((( FieldAnimTaskContext  *)0)->  used ))  == 0x10 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_task_context_slots [( ((u32)&((( FieldAnimTaskContext  *)0)->  slots ))  == 0x20 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_task_context_table [( ((u32)&((( FieldAnimTaskContext  *)0)->  table ))  == 0x80 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_task_context_arena [( ((u32)&((( FieldAnimTaskContext  *)0)->  arena ))  == 0x84 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_task_context_size [( sizeof(FieldAnimTaskContext) == 0xA00 ) ? 1 : -1] ;
extern FieldAnimTaskContext *D_800E2368;
int func_800D401C(int id);
extern FieldAnimTaskSlot *D_800F33E0;
int func_800CE560(char *out, int stride, int count, FieldAnimTaskCallback callback);
void *func_800CE610(char *list);
int func_800CE5AC(void *arg0, int arg1, int arg2, int arg3, void *arg4);
int func_800CE688(char *arg0);
int func_800CE78C(char *arg0);
typedef struct FieldAnimEmitter {
    int angle;
} FieldAnimEmitter;
typedef struct FieldAnimEmittedPoint {
    s16 position[3];
    s16 state;
    s16 value08;
    s16 angle;
    s16 phase;
} FieldAnimEmittedPoint;
typedef struct FieldAnimObjectPrefix {
    u8 reserved00;
    u8 asset_type;
    u8 reserved02[6];
    struct FieldActor *actor;
} FieldAnimObjectPrefix;
typedef char pe1_static_assert_field_anim_object_actor_offset [( ((u32)&((( FieldAnimObjectPrefix  *)0)->  actor ))  == 8 ) ? 1 : -1] ;
typedef struct FieldAnimTaskOwner {
    FieldAnimObjectPrefix prefix;
    FieldAnimTaskContext tasks;
} FieldAnimTaskOwner;
typedef char pe1_static_assert_field_anim_owner_asset_type [( ((u32)&((( FieldAnimObjectPrefix  *)0)->  asset_type ))  == 1 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_owner_tasks [( ((u32)&((( FieldAnimTaskOwner  *)0)->  tasks ))  == 0xC ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_owner_slots [( ((u32)&((( FieldAnimTaskOwner  *)0)->  tasks.slots ))  == 0x2C ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_owner_flags [( ((u32)&((( FieldAnimTaskOwner  *)0)->  tasks.flags ))  == 0x19 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_owner_table [( ((u32)&((( FieldAnimTaskOwner  *)0)->  tasks.table ))  == 0x8C ) ? 1 : -1] ;
int func_800D4704(FieldAnimTaskOwner *owner);
extern FieldAnimObjectPrefix *D_800F32D0;
extern s16 D_800E2214[3];
extern s16 D_800942EC;
int func_800DA1FC(int mode, void *state);
int func_800DA5D4(int mode, FieldAnimEmitter *state);
extern u8 *D_800F32D8;
extern s16 D_800E220C[];
void func_800C6D5C(u8 *data, u8 x_offset, u8 y_offset);
int func_800D9A8C(int mode, void *state);
int func_800DF9B0 (int mode, FieldAnimEmitter *state);
typedef struct FieldAnimPointTriple {
    u16 x;
    u16 y;
    u16 z;
} FieldAnimPointTriple;
typedef struct FieldAnimPointData {
    u8 unused_00[3];
    u8 scale;
    s16 count;
    u16 unused_06;
    u16 x[16];
    u16 y[16];
    u16 z[16];
} FieldAnimPointData;
typedef struct FieldAnimScatteredParticles {
    FieldAnimPointData points;
    s16 velocity_x[16];
    s16 velocity_y[16];
    s16 velocity_z[16];
} FieldAnimScatteredParticles;
typedef char pe1_static_assert_field_anim_point_data_size [( sizeof(FieldAnimPointData) == 0x68 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_scattered_velocity_x [( ((u32)&((( FieldAnimScatteredParticles  *)0)->  velocity_x ))  == 0x68 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_scattered_velocity_y [( ((u32)&((( FieldAnimScatteredParticles  *)0)->  velocity_y ))  == 0x88 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_scattered_velocity_z [( ((u32)&((( FieldAnimScatteredParticles  *)0)->  velocity_z ))  == 0xA8 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_anim_scattered_particles_size [( sizeof(FieldAnimScatteredParticles) == 0xC8 ) ? 1 : -1] ;
void func_800CC2C4(void *arg0, void *arg1, FieldAnimScatteredParticles *state);
typedef struct FieldAnimPointState {
    FieldAnimPointTriple point;
    u8 unused_06[0x22];
    short scale;
} FieldAnimPointState;
typedef struct FieldAnimInterleavedState {
    FieldAnimPointTriple point;
    u8 unused_06[0xA];
    int extent_x;
    int extent_y;
    int extent_z;
    u8 unused_1C[0xC];
    short scale;
} FieldAnimInterleavedState;
typedef struct FieldAnimInterleavedWindow {
    u8 unused_00[0x26];
    FieldAnimPointTriple point;
} FieldAnimInterleavedWindow;
typedef struct FieldAnimBurstWindow {
    u8 unused_00[0x10];
    FieldAnimPointTriple point;
    u8 unused_16[0xA];
    FieldAnimPointTriple offset;
} FieldAnimBurstWindow;
typedef struct FieldAnimBurstHeader {
    u8 unused_00[3];
    u8 mode;
    u8 scale[2];
    u8 duration[2];
    u8 origin_x[2];
    u8 origin_y[2];
    u8 origin_z[2];
} FieldAnimBurstHeader;
typedef union FieldAnimBurstData {
    FieldAnimBurstHeader header;
    u8 bytes[0x30];
} FieldAnimBurstData;
extern FieldAnimInterleavedState D_800E2260;
extern FieldAnimPointState D_800E2818;
extern FieldAnimPointTriple D_800E27F8;
extern int D_800B0E64;
extern u16 D_800E11EA;
extern int D_800E27EC;
extern u16 D_800E2850[];
extern u16 D_800F336C;
extern short D_800F336E, D_800F3372, D_800F3374;
extern u16 D_800F3370;
extern int D_80195EF8;
extern int func_801940B0(int mode, void *state);
extern int func_8006E498(int, int);
extern int func_800D3FD8(void);
extern void func_800D3F64(int, int);
int func_80195320 (int mode, u16 *out);
typedef struct RoomM256Particle {
    u16 frame;
    u16 offset;
    s16 scale;
} RoomM256Particle;
extern char D_801960A0[];
extern char D_80196098[];
extern char D_80195E64[];
extern s16 D_80196094;
extern s16 D_800F336A;
extern u16 D_800E1204[];
extern int D_800F3428;
extern int *D_800BCFA4;
int func_80077DC4(int);
int func_80077AA4(int, int);
void func_800CF844(void *, void *, int, void *, int, int);
void func_800CF3AC(void *, void *, int);
void func_800CEE20(void *, void *, int, int, int, int, int, int, void *);
void func_800D1DEC(void *, void *, int, int);
int func_8019552C(int mode, RoomM256Particle *particle);
int func_80195320 (int mode, u16 *out) {
    char *p, *q, *r;
    register int ret asm("$2");
    char *task;
    u16 value;
    if (mode == 1) goto mode1;
    if (mode < 2) {
        if (mode == 0) goto mode0;
        return 0;
    }
    if (mode == 2) goto mode2;
    return 0;
mode0:
    *out = (u16)D_800E2368->variables[0];
    D_80195EF8 = func_8006E498(D_800B0E64, 0xC5887704);
    func_800C6D5C((u8 *)D_80195EF8, 0, 0);
    if (D_800E2368->flags) {
        p = (char *)D_800F32D0->actor;
        if (p) {
            q = *(char **)p;
            if (q) {
                r = *(char **)(q + 0x18);
                if (*r == 1) *r = 2;
            }
        }
    }
    return func_800CE560(D_800F33E0->end, 0x18, 0x20,
                           func_801940B0);
mode1:
    if (D_800E27EC == mode) {
        ret = D_800E27EC < 8;
        task = func_800CE610(D_800F33E0->end);
        if (task) {
            value = *out;
            *(s16 *)(task + 0x14) = 0;
            *(s16 *)(task + 0x16) = 0;
            *(u16 *)(task + 6) = value;
        }
        func_800D3F64(0x5A6, func_800D3FD8());
        func_800D3F64(0x5BE, 0x80);
        ret = D_800E27EC < 8;
    } else {
        ret = D_800E27EC < 8;
    }
    if (ret) return 0;
    return 2;
mode2:
    {
        unsigned palette_index = D_800E11EA;
        u16 palette;
        palette = D_800E2850[palette_index];
        asm volatile("" ::: "memory");
        D_800F336C = 3;
        D_800F336E = 0;
        D_800F3372 = 0;
        D_800F3374 = 8;
        D_800F3370 = palette;
        asm volatile("" : "=r"(ret) : : "memory");
    }
    return 0;
}
