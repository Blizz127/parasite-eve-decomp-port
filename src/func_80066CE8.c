/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/main/render/Render_DrawSprite.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `Render_DrawSprite` renamed to `func_80066CE8`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
int func_80077DC4(int angle);
int func_80077CF4(int angle);
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
GteMatrix *func_80079754(GteShortVector *angles, GteMatrix *matrix);
GteShortVector *func_80078C34(const GteMatrix *matrix,
                            const GteShortVector *v, GteShortVector *out);
GteVector *func_80078934(const GteMatrix *matrix, const GteVector *v,
                         GteVector *out);
void func_800783E4(void *first, void *second, int first_scale,
                        int second_scale, void *output);
GteMatrix *func_80078CC4(GteMatrix *matrix, const GteVector *scale);
int func_80079FB4(int y, int x);
int func_80078004(int value);
void func_80078134(GteVector *vector, GteVector *unit);
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
typedef struct RenderCameraGeomState {
    unsigned char pad_00[0x1C];
    int bounds_offset;
} RenderCameraGeomState;
typedef struct RenderCameraBounds {
    unsigned char pad_00[0x28];
    signed short width;
    signed short height;
    signed short min_x;
    signed short max_x;
    signed short min_y;
    signed short max_y;
} RenderCameraBounds;
extern GteMatrix D_800BD000;
extern s16 D_800BD002, D_800BD004, D_800BD006, D_800BD008, D_800BD00A;
extern s16 D_800BD00C, D_800BD00E, D_800BD010;
extern int D_800BD014, D_800BD018, D_800BD01C;
int func_80066CE8(void);
extern unsigned int D_800B89F8[];
extern volatile int D_800B0E40;
extern short D_800BCFB4, D_800BCFB6;
int func_80065B70(void *matrix, void *screen);
int func_800677FC(void *buffer, void **end);
int func_800655D4(void);
void func_80079024(int distance);
int func_80068B94(void);
typedef union RenderPackedGeometry {
    struct {
        u16 record12_counts[2];
        u16 record16_counts[2];
        u16 reserved[4];
    } counts;
    u8 bytes[1];
} RenderPackedGeometry;
typedef char pe1_static_assert_render_packed_geometry_header_size [( sizeof(RenderPackedGeometry) == 16 ) ? 1 : -1] ;
void func_800C7AE0(RenderPackedGeometry *geometry,
                                   GteMatrix *matrix, u16 index,
                                   GteShortVector *out);
void func_800CFB7C(GteShortVector *angles, int distance, GteShortVector *out);
void func_800CFAA8(GteShortVector *from, GteShortVector *to,
                                 GteShortVector *out);
typedef union RenderHistoryPoint {
    GteShortVector vector;
    u32 words[2];
} RenderHistoryPoint;
typedef char pe1_static_assert_render_history_point_size [( sizeof(RenderHistoryPoint) == 8 ) ? 1 : -1] ;
void func_800D3AFC(RenderHistoryPoint *history, s16 count,
                                 RenderHistoryPoint *value, int reset);
typedef struct RenderBouncingSprite {
    s16 x, y, z;
    s16 velocity_x, velocity_y, velocity_z;
    s16 duration, angle;
} RenderBouncingSprite;
typedef char pe1_static_assert_render_bouncing_sprite_size [( sizeof(RenderBouncingSprite) == 16 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_bouncing_sprite_velocity [( ((u32)&((( RenderBouncingSprite  *)0)->  velocity_x ))  == 6 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_bouncing_sprite_duration [( ((u32)&((( RenderBouncingSprite  *)0)->  duration ))  == 12 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_bouncing_sprite_angle [( ((u32)&((( RenderBouncingSprite  *)0)->  angle ))  == 14 ) ? 1 : -1] ;
int func_800D5010(int mode, RenderBouncingSprite *state);
int func_800D6C58(int mode, RenderBouncingSprite *state);
int func_800D4EA4(int mode);
extern u8 D_800E1518[], D_800E1540[];
typedef struct RenderCosineEffect {
    s16 x;
    s16 amplitude;
    s16 y;
    s16 duration;
} RenderCosineEffect;
typedef char pe1_static_assert_render_cosine_effect_size [( sizeof(RenderCosineEffect) == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_cosine_effect_duration [( ((u32)&((( RenderCosineEffect  *)0)->  duration ))  == 6 ) ? 1 : -1] ;
int func_800D5898(int mode, RenderCosineEffect *state);
typedef struct RenderEffectParameters {
    u16 parameter00;
    u16 parameter02;
    u16 palette;
    u16 parameter06;
    u16 tpage;
    u16 parameter0A;
    s16 depth;
    u16 extent_x;
    u16 extent_y;
} RenderEffectParameters;
typedef char pe1_static_assert_render_effect_parameters_size [( sizeof(RenderEffectParameters) == 0x12 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_effect_parameters_palette [( ((u32)&((( RenderEffectParameters  *)0)->  palette ))  == 4 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_effect_parameters_depth [( ((u32)&((( RenderEffectParameters  *)0)->  depth ))  == 0xC ) ? 1 : -1] ;
extern RenderEffectParameters D_800F3368;
typedef struct RenderArcingEmitter {
    GteShortVector position;
    int phase;
    int radius;
} RenderArcingEmitter;
typedef char pe1_static_assert_render_arcing_emitter_size [( sizeof(RenderArcingEmitter) == 0x10 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_arcing_emitter_radius [( ((u32)&((( RenderArcingEmitter  *)0)->  radius ))  == 0xC ) ? 1 : -1] ;
int func_800D7B70(int mode, RenderArcingEmitter *state);
extern u8 D_800E18F0[];
extern u16 D_800E11E6;
extern u16 D_800E11F6;
extern u16 D_800E2850[];
void func_800CEDA8(int index);
typedef struct RenderOrbitingEffect {
    s16 stage;
    s16 angle;
    s16 timer;
    s16 x, y, z;
    s16 radius;
    s16 reserved0E;
} RenderOrbitingEffect;
typedef char pe1_static_assert_render_orbiting_effect_size [( sizeof(RenderOrbitingEffect) == 0x10 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_orbiting_effect_position [( ((u32)&((( RenderOrbitingEffect  *)0)->  x ))  == 6 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_orbiting_effect_radius [( ((u32)&((( RenderOrbitingEffect  *)0)->  radius ))  == 0xC ) ? 1 : -1] ;
int func_800D7FBC(int mode, RenderOrbitingEffect *state);
typedef struct RenderOrbitingEmitter {
    int phase;
    char *particles;
} RenderOrbitingEmitter;
typedef char pe1_static_assert_render_orbiting_emitter_size [( sizeof(RenderOrbitingEmitter) == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_orbiting_emitter_particles [( ((u32)&((( RenderOrbitingEmitter  *)0)->  particles ))  == 4 ) ? 1 : -1] ;
int func_800D868C(int mode, RenderOrbitingEmitter *state);
extern GteShortVector D_800E21EC;
extern char *D_800E21F4;
typedef struct RenderVerticalEffect {
    s16 stage;
    s16 angle;
    s16 timer;
    s16 x, y, z;
} RenderVerticalEffect;
typedef char pe1_static_assert_render_vertical_effect_size [( sizeof(RenderVerticalEffect) == 0xC ) ? 1 : -1] ;
typedef char pe1_static_assert_render_vertical_effect_timer [( ((u32)&((( RenderVerticalEffect  *)0)->  timer ))  == 4 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_vertical_effect_position [( ((u32)&((( RenderVerticalEffect  *)0)->  x ))  == 6 ) ? 1 : -1] ;
int func_800D8978(int mode, RenderVerticalEffect *state);
int func_800D8B6C(int mode, void *state);
extern GteShortVector D_800E21F8;
extern u8 D_800E1A14[];
typedef struct RenderHelicalEffect {
    s16 stage;
    s16 angle;
    s16 timer;
    s16 x, y, z;
    s16 radius;
    s16 direction;
} RenderHelicalEffect;
typedef char pe1_static_assert_render_helical_effect_size [( sizeof(RenderHelicalEffect) == 0x10 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_helical_effect_position [( ((u32)&((( RenderHelicalEffect  *)0)->  x ))  == 6 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_helical_effect_direction [( ((u32)&((( RenderHelicalEffect  *)0)->  direction ))  == 0xE ) ? 1 : -1] ;
int func_800D8D14(int mode, GteShortVector *state);
int func_800D8E74(int mode, RenderHelicalEffect *state);
int func_800D927C(int mode, RenderOrbitingEmitter *state);
extern u8 D_800E1AA0[];
extern GteShortVector D_800E2200;
extern char *D_800E2208;
typedef struct RenderDampedSpark {
    s16 x, y, z;
    s16 vx, vy, vz;
} RenderDampedSpark;
typedef struct RenderSparkEmitter {
    GteShortVector position;
    int phase;
} RenderSparkEmitter;
typedef char pe1_static_assert_render_damped_spark_size [( sizeof(RenderDampedSpark) == 0xC ) ? 1 : -1] ;
typedef char pe1_static_assert_render_damped_spark_velocity [( ((u32)&((( RenderDampedSpark  *)0)->  vx ))  == 6 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_spark_emitter_size [( sizeof(RenderSparkEmitter) == 0xC ) ? 1 : -1] ;
typedef char pe1_static_assert_render_spark_emitter_phase [( ((u32)&((( RenderSparkEmitter  *)0)->  phase ))  == 8 ) ? 1 : -1] ;
int func_800D9554(int mode, RenderDampedSpark *state);
int func_800D96F4(int mode, RenderSparkEmitter *state);
void func_800D1DEC(void *position, void *color, int scale, int flags);
extern u8 D_800E1AC8[];
typedef struct RenderArcingEffect {
    s16 x, y, z;
    s16 velocity_y;
} RenderArcingEffect;
typedef char pe1_static_assert_render_arcing_effect_size [( sizeof(RenderArcingEffect) == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_arcing_effect_velocity [( ((u32)&((( RenderArcingEffect  *)0)->  velocity_y ))  == 6 ) ? 1 : -1] ;
int func_800D7A1C(int mode, RenderArcingEffect *state);
int func_800D9E5C(int mode, RenderArcingEffect *state);
int func_800DA780(int mode, RenderArcingEffect *state);
int func_800DA934(int mode, GteShortVector *state);
int func_800D9FD4(int mode, GteShortVector *state);
extern u8 D_800E18C0[];
typedef struct RenderSineEffect {
    GteShortVector position;
    s16 angle;
    s16 scale;
    s16 amplitude;
    s16 velocity_y;
} RenderSineEffect;
typedef char pe1_static_assert_render_sine_effect_size [( sizeof(RenderSineEffect) == 0x10 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_sine_effect_angle [( ((u32)&((( RenderSineEffect  *)0)->  angle ))  == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_sine_effect_amplitude [( ((u32)&((( RenderSineEffect  *)0)->  amplitude ))  == 0xC ) ? 1 : -1] ;
typedef char pe1_static_assert_render_sine_effect_velocity [( ((u32)&((( RenderSineEffect  *)0)->  velocity_y ))  == 0xE ) ? 1 : -1] ;
typedef struct RenderSineEmitter {
    GteShortVector position;
    int phase;
} RenderSineEmitter;
typedef char pe1_static_assert_render_sine_emitter_size [( sizeof(RenderSineEmitter) == 0xC ) ? 1 : -1] ;
typedef char pe1_static_assert_render_sine_emitter_phase [( ((u32)&((( RenderSineEmitter  *)0)->  phase ))  == 8 ) ? 1 : -1] ;
int func_800DB0D0(int mode, RenderSineEmitter *state);
extern u8 D_800E1C2C[];
extern GteShortVector D_800E221C;
int func_800DAF8C(int mode, RenderSineEffect *state);
void func_800D0E88(void *data, GteShortVector *position, int scale, int angle,
                   void *color, int arg5, int arg6, int intensity, int mode);
extern int D_800E27EC;
extern u8 D_800E1C04[];
extern int D_800E1D60;
extern u8 D_800E1D64[], D_800E1D84[];
int func_800DBA9C(int mode, RenderSparkEmitter *state);
extern u8 D_800E1DA4[];
extern u8 D_800E1E64[];
int func_800DC5BC(int mode, GteShortVector *state);
int func_800DC750(int mode, GteShortVector *state);
int func_800DBCD8(int mode, GteShortVector *state);
int func_800DBE6C(int mode, GteShortVector *state);
void func_800CF3AC(void *track, void *color, int time);
void func_800D27FC(int x, int y, void *color, int scale, int mode);
int func_800DAB98(int mode, RenderCosineEffect *effect);
extern s16 D_800F3374;
extern u8 D_800E1494[];
void func_800CE870(char *object, int mode, s16 *position);
void func_800D0728(GteShortVector *position, int arg1, int arg2, int arg3,
                   GteRotation *rotation, int scale_x, int scale_y,
                   void *data, void *color, int intensity, int mode);
int func_800DF87C(int mode);
typedef struct RenderColor {
    u8 r, g, b, code;
} RenderColor;
extern RenderColor D_800C22C0;
extern RenderColor D_800C22D0;
typedef struct RenderDiamondEmitter {
    GteShortVector position;
    int x, y, angle, size, radius;
} RenderDiamondEmitter;
extern GteShortVector D_800E2224;
extern u8 D_800E1CC8[];
typedef struct RenderDiamondParticle {
    s16 x, y, angle, size, reserved, color_time;
} RenderDiamondParticle;
typedef char pe1_static_assert_render_diamond_emitter_size [( sizeof(RenderDiamondEmitter) == 28 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_diamond_emitter_radius [( ((u32)&((( RenderDiamondEmitter  *)0)->  radius ))  == 24 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_diamond_particle_size [( sizeof(RenderDiamondParticle) == 12 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_diamond_particle_color_time [( ((u32)&((( RenderDiamondParticle  *)0)->  color_time ))  == 10 ) ? 1 : -1] ;
int func_800DB5F4(int mode, RenderDiamondParticle *state);
int func_800DB6BC(int mode, RenderDiamondEmitter *state);
typedef struct RenderSettlingSprite {
    GteShortVector position;
    s16 stage, timer, phase, reserved;
} RenderSettlingSprite;
typedef char pe1_static_assert_render_settling_sprite_size [( sizeof(RenderSettlingSprite) == 16 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_settling_sprite_stage [( ((u32)&((( RenderSettlingSprite  *)0)->  stage ))  == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_settling_sprite_phase [( ((u32)&((( RenderSettlingSprite  *)0)->  phase ))  == 12 ) ? 1 : -1] ;
extern GteShortVector D_800E223C;
extern u8 D_800E20AC[];
int func_800DD9E4(int mode, RenderSettlingSprite *state);
int func_800DDD70(int mode, GteShortVector *state);
typedef struct RenderConvergingSprite {
    GteShortVector position;
    s16 stage, timer, bursts, phase;
} RenderConvergingSprite;
typedef struct RenderConvergingEmitter {
    GteShortVector position, target;
} RenderConvergingEmitter;
typedef char pe1_static_assert_render_converging_sprite_size [( sizeof(RenderConvergingSprite) == 16 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_converging_sprite_stage [( ((u32)&((( RenderConvergingSprite  *)0)->  stage ))  == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_converging_sprite_phase [( ((u32)&((( RenderConvergingSprite  *)0)->  phase ))  == 14 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_converging_emitter_size [( sizeof(RenderConvergingEmitter) == 16 ) ? 1 : -1] ;
extern GteShortVector D_800E2234;
extern u8 D_800E1FEC[];
int func_800DD380(int mode, RenderConvergingSprite *state);
int func_800DD76C(int mode, RenderConvergingEmitter *state);
typedef struct RenderTiltingSprite {
    GteRotation rotation;
    s16 height, width;
    s16 initial_tilt, velocity_y;
} RenderTiltingSprite;
typedef char pe1_static_assert_render_tilting_sprite_size [( sizeof(RenderTiltingSprite) == 16 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tilting_sprite_height [( ((u32)&((( RenderTiltingSprite  *)0)->  height ))  == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tilting_sprite_initial_tilt [( ((u32)&((( RenderTiltingSprite  *)0)->  initial_tilt ))  == 12 ) ? 1 : -1] ;
extern u8 D_800E1F18[];
extern GteShortVector D_800E222C;
int func_800DCCCC(int mode, RenderTiltingSprite *state);
int func_800DCE94(int mode, RenderSparkEmitter *state);
void func_800D2370(GteShortVector *position, GteRotation *rotation,
                   int width, int height, int u, int v, int texture_width,
                   int texture_height, int clut, RenderColor *color0,
                   RenderColor *color1, int intensity, int mode);
extern RenderColor D_800C22DC;
extern RenderColor D_800C22E0;
extern RenderColor D_800C22E4;
extern RenderColor D_800C22E8;
extern RenderColor D_800C22EC;
void func_800D004C(GteShortVector *position, int width, int height, int segments,
                   GteRotation *rotation, int scale_x, int scale_y,
                   RenderColor *color0, RenderColor *color1, int intensity, int mode);
int func_800DACA4(int mode, RenderSparkEmitter *state);
typedef char pe1_static_assert_render_color_size [( sizeof(RenderColor) == 4 ) ? 1 : -1] ;
typedef struct RenderColorKeyTiming {
    u16 length;
    u16 start;
} RenderColorKeyTiming;
typedef struct RenderColorKey {
    RenderColor color;
    RenderColorKeyTiming timing;
} RenderColorKey;
typedef struct RenderColorTrack {
    int duration;  
    int count;
    RenderColorKey keys[0];
} RenderColorTrack;
typedef char pe1_static_assert_render_color_key_size [( sizeof(RenderColorKey) == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_color_key_timing_offset [( ((u32)&((( RenderColorKey  *)0)->  timing ))  == 4 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_color_track_header_size [( sizeof(RenderColorTrack) == 8 ) ? 1 : -1] ;
void func_80078554(void *first, void *second, int first_scale,
                    int second_scale, void *output);
extern u8 D_800E1988[];
extern u8 D_800E1EE8[];
extern u16 D_800F336C;
extern u16 D_800E1204[];
extern int D_800F3428;
void func_800CEE20(GteShortVector *position, GteRotation *rotation,
                   int scale_x, int scale_y, int texture, int clut,
                   int page, int intensity, RenderColor *color);
int func_800DC910(int mode, GteShortVector *position);
int func_800DCA80(int mode, GteShortVector *position);
extern u8 D_800E1FA4[];
extern u8 D_800E1FCC[];
int func_800DD19C(int mode, RenderSparkEmitter *state);
int func_800D7E78(int mode, GteShortVector *state);
void func_800CEAE8(const GteMatrixWords *matrix,
                           const GteShortVector *input, GteShortVector *output);
typedef struct RenderMatrixSlot {
    s32 *value;
    u8 reserved[8];
} RenderMatrixSlot;
extern RenderMatrixSlot D_800BCFA4;
void func_800CF5B0(const GteShortVector *input,
                                  GteMatrixWords *output);
struct RoomFxTransformOwner;
extern GteShortVector D_800C2258, D_800C2260;
void func_800CE9D4(struct RoomFxTransformOwner *owner, int index,
                   GteShortVector *out);
void func_800CE8F0(struct RoomFxTransformOwner *owner, int index,
                                  const GteShortVector *input,
                                  GteShortVector *output);
typedef struct RenderVec3s {
    signed short x;
    signed short y;
    signed short z;
    signed short pad;
} RenderVec3s;
typedef struct RenderObjectPart {
    unsigned short vertex_start;
    unsigned short vertex_count;
    unsigned char visible;
    unsigned char pad_05[3];
    s32 translation_z;
} RenderObjectPart;
typedef char pe1_static_assert_render_object_part_size [( sizeof(RenderObjectPart) == 12 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_part_translation_z [( ((u32)&((( RenderObjectPart  *)0)->  translation_z ))  == 8 ) ? 1 : -1] ;
typedef struct RenderObjectHeader {
    unsigned char pad_00[2];
    unsigned char part_count;
    unsigned char animation_entry_count;
    unsigned char pad_04[4];
    unsigned short packet34_count;
    unsigned short packet28_count;
    unsigned short packet24_count;
    unsigned short packet1c_count;
    u16 anchor_y;
    u16 anchor_matrix_index;
    unsigned char pad_14[6];
    unsigned short visible_part_count;
} RenderObjectHeader;
typedef struct RenderPrimitiveDescriptor {
      unsigned char reserved00[3];
      u8 kind;
      u16 lookup_indices[4];
} RenderPrimitiveDescriptor;
typedef struct RenderPacketState {
      unsigned char reserved00[0x1A];
      u16 page_bits;
} RenderPacketState;
typedef union RenderPacketValue {
    u32 value;
    struct {
        u8 byte0;
        u8 byte1;
        u8 byte2;
        u8 command;
    } bytes;
} RenderPacketValue;
typedef struct RenderPacket34 {
      u32 tag;
      RenderPacketValue values0;
      u32 sxy0;
      u8 u0, v0;
      u16 clut;
      u32 value1;
      u32 sxy1;
      u8 u1, v1;
      u16 page_bits;
      u32 value2;
      u32 sxy2;
      u8 u2, v2;
      u16 reserved26;
      u32 value3;
      u32 sxy3;
      u8 u3, v3;
      u16 reserved32;
} RenderPacket34;
typedef struct RenderPacket28 {
      u32 tag;
      RenderPacketValue values0;
      u32 sxy0;
      u8 u0, v0;
      u16 clut;
      u32 value1;
      u32 sxy1;
      u8 u1, v1;
      u16 page_bits;
      u32 value2;
      u32 sxy2;
      u8 u2, v2;
      u16 reserved26;
} RenderPacket28;
typedef struct RenderPacket24 {
      u32 tag;
      RenderPacketValue values0;
      u32 sxy0;
      u32 value1;
      u32 sxy1;
      u32 value2;
      u32 sxy2;
      u32 value3;
      u32 sxy3;
} RenderPacket24;
typedef struct RenderPacket1C {
      u32 tag;
      RenderPacketValue values0;
      u32 sxy0;
      u32 value1;
      u32 sxy1;
      u32 value2;
      u32 sxy2;
} RenderPacket1C;
typedef struct RenderAnimationLookupEntry {
      s16 value0;
      s16 value1;
      s16 value2;
      s16 animation_id;
      unsigned char reserved08[4];
} RenderAnimationLookupEntry;
typedef struct RenderAnimationDataHeader {
      u8 encoding_flags;
      u8 last_bone_index;
      u8 packing_flags;
      u8 reserved03;
      u16 object_value74;
      u16 object_value76;
      u16 object_value78;
      u16 object_value7c;
} RenderAnimationDataHeader;
typedef struct RenderAnimByteChannel {
      u8 constant_marker;
      u8 value_or_samples[1];
      u8 reserved02[2];
} RenderAnimByteChannel;
typedef struct RenderAnimShortChannel {
      s16 constant_marker;
      union {
        s16 signed_values[1];
        u16 unsigned_values[1];
    } samples;
} RenderAnimShortChannel;
typedef struct RenderRotationOverride {
      u16 x;
      u16 y;
      u16 z;
      u8 matrix_index;
      u8 flags;
} RenderRotationOverride;
typedef struct RenderMatrix {
      s16 rotation[3][3];
      s16 reserved12;
      s32 translation[3];
} RenderMatrix;
typedef struct RenderObjectEntity {
      RenderObjectHeader *header;
      RenderObjectPart *parts;
      RenderVec3s *vertices;
      unsigned int *vertex_colours;
      RenderPrimitiveDescriptor *primitive_descriptors;
      void *model_section14;
      RenderVec3s *bounds_vertices;
      RenderVec3s *projection_origin;
      s8 *matrix_commands;
      struct RenderObjectEntity *animation_source;
      s16 animation_state;
      s16 animation_id;
      u16 table_value2c;
      u16 table_value2e;
      u16 table_value30;
      u16 table_index;
      RenderMatrix model_matrix;
      u8 *primitive_buffer;
      RenderMatrix *active_matrix;
      s16 projected_x;
      s16 projected_y;
      unsigned char pad_60[4];
      s16 projected_target_x, projected_target_y;
      RenderVec3s anchor_position;
      u16 table_value70;
      unsigned char pad_72[2];
      u16 animation_value74;
      u16 animation_value76;
      u16 animation_value78;
      u16 reserved7a;
      u16 animation_value7c;
      u16 reserved7e;
      RenderAnimationLookupEntry *animation_entries;
      RenderMatrix *matrices;
      unsigned char shade;
      u8 lightNegativeY;
      u8 lightPositiveY;
      u8 reserved8b;
      s8 fade_remaining, fade_duration;
      u8 fade_red_step, fade_green_step;
      u8 primitive_red, primitive_green, primitive_blue;
      u8 fade_blue_step;
      u8 fade_red, fade_green, fade_blue;
      u8 script_param97;
      u8 script_param98;
      u8 script_param99;
      s16 script_value9a;
      u16 flags_9C;
      u8 variant_visible;
      unsigned char reserved9f;
      RenderRotationOverride rotation_overrides[2];
      void *animation_data;
      s16 target_x;
      s16 target_y;
      s16 target_z;
      signed short draw_count;
} RenderObjectEntity;
typedef char pe1_static_assert_render_matrix_size [( sizeof(RenderMatrix) == 0x20 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_primitive_descriptor_kind_offset [( ((u32)&((( RenderPrimitiveDescriptor  *)0)->  kind ))  == 3 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_primitive_descriptor_size [( sizeof(RenderPrimitiveDescriptor) == 0x0C ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet_state_prefix_size [( sizeof(RenderPacketState) == 0x1C ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet_value_size [( sizeof(RenderPacketValue) == 0x04 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet34_size [( sizeof(RenderPacket34) == 0x34 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet28_size [( sizeof(RenderPacket28) == 0x28 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet34_clut [( ((u32)&((( RenderPacket34  *)0)->  clut ))  == 0x0E ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet34_uv1 [( ((u32)&((( RenderPacket34  *)0)->  u1 ))  == 0x18 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet34_uv2 [( ((u32)&((( RenderPacket34  *)0)->  u2 ))  == 0x24 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet34_v3 [( ((u32)&((( RenderPacket34  *)0)->  v3 ))  == 0x31 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet28_clut [( ((u32)&((( RenderPacket28  *)0)->  clut ))  == 0x0E ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet28_uv1 [( ((u32)&((( RenderPacket28  *)0)->  u1 ))  == 0x18 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet28_uv2 [( ((u32)&((( RenderPacket28  *)0)->  u2 ))  == 0x24 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet24_size [( sizeof(RenderPacket24) == 0x24 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet1c_size [( sizeof(RenderPacket1C) == 0x1C ) ? 1 : -1] ;
typedef char pe1_static_assert_render_animation_data_header_size [( sizeof(RenderAnimationDataHeader) == 0x0C ) ? 1 : -1] ;
typedef char pe1_static_assert_render_anim_byte_channel_size [( sizeof(RenderAnimByteChannel) == 0x04 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_anim_short_channel_size [( sizeof(RenderAnimShortChannel) == 0x04 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_rotation_override_size [( sizeof(RenderRotationOverride) == 0x08 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_header_primitive_counts_offset [( ((u32)&((( RenderObjectHeader  *)0)->  packet34_count ))  == 0x08 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_matrix_translation_offset [( ((u32)&((( RenderMatrix  *)0)->  translation ))  == 0x14 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_model_matrix_offset [( ((u32)&((( RenderObjectEntity  *)0)->  model_matrix ))  == 0x34 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_matrices_offset [( ((u32)&((( RenderObjectEntity  *)0)->  matrices ))  == 0x84 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_animation_lookup_entry_size [( sizeof(RenderAnimationLookupEntry) == 0x0C ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_animation_data_offset [( ((u32)&((( RenderObjectEntity  *)0)->  animation_data ))  == 0xB0 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_target_x_offset [( ((u32)&((( RenderObjectEntity  *)0)->  target_x ))  == 0xB4 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_target_z_offset [( ((u32)&((( RenderObjectEntity  *)0)->  target_z ))  == 0xB8 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_primitive_red_offset [( ((u32)&((( RenderObjectEntity  *)0)->  primitive_red ))  == 0x90 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_primitive_green_offset [( ((u32)&((( RenderObjectEntity  *)0)->  primitive_green ))  == 0x91 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_primitive_blue_offset [( ((u32)&((( RenderObjectEntity  *)0)->  primitive_blue ))  == 0x92 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_fade_remaining_offset [( ((u32)&((( RenderObjectEntity  *)0)->  fade_remaining ))  == 0x8C ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_fade_duration_offset [( ((u32)&((( RenderObjectEntity  *)0)->  fade_duration ))  == 0x8D ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_fade_red_step_offset [( ((u32)&((( RenderObjectEntity  *)0)->  fade_red_step ))  == 0x8E ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_fade_green_step_offset [( ((u32)&((( RenderObjectEntity  *)0)->  fade_green_step ))  == 0x8F ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_fade_blue_step_offset [( ((u32)&((( RenderObjectEntity  *)0)->  fade_blue_step ))  == 0x93 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_fade_red_offset [( ((u32)&((( RenderObjectEntity  *)0)->  fade_red ))  == 0x94 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_fade_green_offset [( ((u32)&((( RenderObjectEntity  *)0)->  fade_green ))  == 0x95 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_fade_blue_offset [( ((u32)&((( RenderObjectEntity  *)0)->  fade_blue ))  == 0x96 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_header_anchor_y [( ((u32)&((( RenderObjectHeader  *)0)->  anchor_y ))  == 0x10 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_header_anchor_matrix_index [( ((u32)&((( RenderObjectHeader  *)0)->  anchor_matrix_index ))  == 0x12 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_projection_origin [( ((u32)&((( RenderObjectEntity  *)0)->  projection_origin ))  == 0x1C ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_projected_target [( ((u32)&((( RenderObjectEntity  *)0)->  projected_target_x ))  == 0x64 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_anchor_position [( ((u32)&((( RenderObjectEntity  *)0)->  anchor_position ))  == 0x68 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_object_entity_size [( sizeof(RenderObjectEntity) == 0xBC ) ? 1 : -1] ;
union RenderLightingMatrix;
void func_8003B97C(RenderObjectEntity *object, union RenderLightingMatrix *matrix);
extern u32 D_8009CDA0;
int func_8003C638(RenderObjectEntity *object);
int func_8003C818(RenderObjectEntity *object);
void func_8003CEF8(RenderObjectEntity *object, int mode);
void func_8003CCB0(RenderObjectEntity *object, int mode);
void func_8003CAEC(RenderObjectEntity *object, int r, int g, int b);
void func_8003E474(RenderObjectEntity *object,
                                           int du, int dv, int clutOffset);
void Render_SetObjectAnim(RenderObjectEntity *object, RenderObjectEntity *source,
                          short animation_id);
void Render_ClearObjectAnim(RenderObjectEntity *object);
void func_8003E188(RenderObjectEntity *object, s32 *view_matrix);
void func_8003A6A8(RenderObjectEntity *object, u32 *view_matrix);
void func_8003AC90(RenderObjectEntity *object, u32 *view_matrix);
extern u32 D_800A6360[], D_800B1638[];
extern u8 D_8009CD98[];
extern s16 D_8009CD9A;
void func_8003DBE4(RenderObjectEntity *dst, RenderObjectEntity *src, s32 frame);
void func_8003DD08(RenderObjectEntity *dst, RenderObjectEntity *src, s32 frame);
void func_8003BCE0(RenderObjectEntity *object, s16 force, s16 buffer_index);
void Anim_DecodeBoneRotationsByte(RenderObjectEntity *object, RenderAnimationDataHeader *animation_data,
                                  s16 frame);
void Anim_DecodeBoneRotationsShort(RenderObjectEntity *object, RenderAnimationDataHeader *animation_data,
                                   s16 frame);
void func_800347B4(void);
typedef char pe1_static_assert_render_packet34_sxy0 [( ((u32)&((( RenderPacket34  *)0)->  sxy0 ))  == 0x08 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet34_sxy1 [( ((u32)&((( RenderPacket34  *)0)->  sxy1 ))  == 0x14 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet34_sxy2 [( ((u32)&((( RenderPacket34  *)0)->  sxy2 ))  == 0x20 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet34_sxy3 [( ((u32)&((( RenderPacket34  *)0)->  sxy3 ))  == 0x2C ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet28_sxy0 [( ((u32)&((( RenderPacket28  *)0)->  sxy0 ))  == 0x08 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet28_sxy1 [( ((u32)&((( RenderPacket28  *)0)->  sxy1 ))  == 0x14 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet28_sxy2 [( ((u32)&((( RenderPacket28  *)0)->  sxy2 ))  == 0x20 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet24_sxy0 [( ((u32)&((( RenderPacket24  *)0)->  sxy0 ))  == 0x08 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet24_sxy1 [( ((u32)&((( RenderPacket24  *)0)->  sxy1 ))  == 0x10 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet24_sxy2 [( ((u32)&((( RenderPacket24  *)0)->  sxy2 ))  == 0x18 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet24_sxy3 [( ((u32)&((( RenderPacket24  *)0)->  sxy3 ))  == 0x20 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet1c_sxy0 [( ((u32)&((( RenderPacket1C  *)0)->  sxy0 ))  == 0x08 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet1c_sxy1 [( ((u32)&((( RenderPacket1C  *)0)->  sxy1 ))  == 0x10 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_packet1c_sxy2 [( ((u32)&((( RenderPacket1C  *)0)->  sxy2 ))  == 0x18 ) ? 1 : -1] ;
typedef struct RenderRisingEffect {
    u16 x, y, z;
    u16 vx, vy, vz;
    u16 angle;
} RenderRisingEffect;
typedef char pe1_static_assert_render_rising_effect_size [( sizeof(RenderRisingEffect) == 14 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_rising_effect_angle [( ((u32)&((( RenderRisingEffect  *)0)->  angle ))  == 12 ) ? 1 : -1] ;
extern u8 D_800E1694[];
extern s16 D_800F336A;
int func_800D5CE4(int mode, RenderRisingEffect *state);
typedef struct RenderFadeEmitter {
    s16 count, intensity;
} RenderFadeEmitter;
typedef struct RenderFadeParticle {
    s16 position[3];
    s16 kind;
    s16 reserved[4];
    s16 phase, timer;
} RenderFadeParticle;
typedef char pe1_static_assert_render_fade_emitter_size [( sizeof(RenderFadeEmitter) == 4 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_fade_particle_size [( sizeof(RenderFadeParticle) == 20 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_fade_particle_kind [( ((u32)&((( RenderFadeParticle  *)0)->  kind ))  == 6 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_fade_particle_phase [( ((u32)&((( RenderFadeParticle  *)0)->  phase ))  == 16 ) ? 1 : -1] ;
extern u8 D_800E2164[];
extern u16 D_800E2244;
int func_800DEFFC(int mode, void *state);
int func_800DF6AC(int mode, RenderFadeEmitter *state);
typedef struct FieldActorState FieldActorState;
typedef struct FieldActor {
      FieldActorState *state;   
      struct FieldActor *next;    
      struct FieldActor *prev;    
      unsigned char type_id;      
      unsigned char sub_id;
      unsigned char mode;
      unsigned char action;
      unsigned char pad_010[0x02];
      unsigned short anim_frame_target;  
      union {
        int fixed;                                 
        struct { unsigned short fraction; unsigned short integer; } parts;
    } anim;
      int anim_prev;                     
      int anim_step;                     
      int move_factor;           
      unsigned short field_sfx_id;  
      unsigned short move_speed;   
      int pos_x;
      int pos_y;
      int pos_z;
      unsigned char pad_034[0x04];
      short rot_x;
      short rot_y;    
      short rot_z;
      unsigned char pad_03E[0x02];
      int base_x;              
      int base_y;
      int base_z;
      int field_4c;            
      short saved_rot_x;
      short saved_rot_y;
      short saved_rot_z;
      unsigned char pad_056[0x02];
      int delta_x;             
      int delta_y;
      int delta_z;
      unsigned char pad_064[0x04];
      int motion_x;
      int motion_y;
      int motion_z;
      unsigned char pad_074[0x04];
      int accel_x;    
      int accel_y;
      int accel_z;
      unsigned char pad_084[0x04];
      int gravity_x;  
      int gravity_y;
      int gravity_z;
      unsigned char pad_094[0x04];
      unsigned int flags;
      unsigned char *script_base;  
      struct FieldActorNode *task_node_lists[3];  
      unsigned char pad_0AC[0xE0];
      struct FieldActor *parent;  
      unsigned char pad_190[0x0C];
      int script_cursor_19c;
      int script_cursor_1a0;
      int field_1a4;           
      int field_1a8;
      int allocation_active;  
      void *action_data;       
      RenderObjectEntity render_object;
      unsigned char pad_270[0x08];
      int allocation_block;
} FieldActor;
typedef char pe1_static_assert_field_actor_prev_offset [( ((u32)&((( FieldActor  *)0)->  prev ))  == 0x08 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_actor_anim_offset [( ((u32)&((( FieldActor  *)0)->  anim ))  == 0x14 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_actor_script_base_offset [( ((u32)&((( FieldActor  *)0)->  script_base ))  == 0x9C ) ? 1 : -1] ;
typedef char pe1_static_assert_field_actor_allocation_active_offset [( ((u32)&((( FieldActor  *)0)->  allocation_active ))  == 0x1AC ) ? 1 : -1] ;
typedef char pe1_static_assert_field_actor_render_scale_offset [( ((u32)&((( FieldActor  *)0)->  render_object.table_value70 ))  == 0x224 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_actor_render_flags_offset [( ((u32)&((( FieldActor  *)0)->  render_object.flags_9C ))  == 0x250 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_actor_allocation_block_offset [( ((u32)&((( FieldActor  *)0)->  allocation_block ))  == 0x278 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_actor_size [( sizeof(FieldActor) == 0x27C ) ? 1 : -1] ;
struct FieldActorState {
      unsigned int core_flags;
      union {
        short value;
        struct { signed char low; signed char high; } bytes;
    } id04;
      union {
        short value;
        struct { signed char low; signed char high; } bytes;
    } id06;
      int progress;
      short amount;
      unsigned short amount_mirror;
      union {
        int command_value;
        struct {
            short actor_threshold;
            unsigned char saved_actor_mode;
            unsigned char reserved13;
        } parts;
    } control10;
      int action_value14;
      int action_value18;
      int count_limit;
      int divisor_basis;
      short actor_threshold_delta;
      unsigned char pad_26[0x02];
      int progress_target;
      int progress_step;
      int countdown;
      int terminal_delay;
      unsigned short sub_action_step;
      unsigned char sub_action_counter;
      unsigned char sub_action_period;
      unsigned char transition_timers[0x0C];
      signed char recoil_steps;
      unsigned char recoil_magnitude;
      short recoil_angle;
      unsigned int flags;
      unsigned char number_desc_0[0x08];
      unsigned char number_desc_1[0x08];
      unsigned char coord_desc[0x08];
      void *action;
      void *substate;
      void *aux_state;
      unsigned char reserved74[0x14];
      int script_value88;
      short script_value8c;
      short script_value8e;
      unsigned char effect_chance;
      unsigned char effect_param91;
      unsigned char effect_param92;
      unsigned char effect_param93;
      unsigned char effect_level;
      unsigned char effect_duration;
      short script_value96;
      short script_value98;
      short script_value9a;
      unsigned char reserved9c[2];
      signed char script_value9e;
      signed char script_value9f;
      short loot_item_id;
      short loot_item_aux;
      signed char behavior_mode;
      unsigned char reserveda5;
      short behavior_timer;
      unsigned char reserveda8[6];
      signed char script_valueae;
      signed char script_valueaf;
      short script_values_b0[6];
      signed char script_valuebc;
      unsigned char reservedbd[0x0F];
      unsigned int status_flags2;
      short panel_c_value;
      unsigned short panel_c_x;
      unsigned short panel_c_y;
      unsigned char panel_c_timer;
      unsigned char panel_c_mode;
};
typedef char pe1_static_assert_field_actor_state_size [( sizeof(FieldActorState) == 0xD8 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_actor_state_pointer_offset [( ((u32)&((( FieldActor  *)0)->  state ))  == 0x00 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_actor_state_script_value88_offset [( ((u32)&((( FieldActorState  *)0)->  script_value88 ))  == 0x88 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_actor_state_behavior_mode_offset [( ((u32)&((( FieldActorState  *)0)->  behavior_mode ))  == 0xA4 ) ? 1 : -1] ;
typedef char pe1_static_assert_field_actor_state_status_flags2_offset [( ((u32)&((( FieldActorState  *)0)->  status_flags2 ))  == 0xCC ) ? 1 : -1] ;
typedef char pe1_static_assert_field_actor_state_panel_c_timer_offset [( ((u32)&((( FieldActorState  *)0)->  panel_c_timer ))  == 0xD6 ) ? 1 : -1] ;
typedef struct FieldMoveTarget {
    unsigned char pad00[0x60];
    int target_pos[3];            
    unsigned char pad6C[0x18];
    FieldActor *target_actor;     
    unsigned char pad88[0x10];
    int completion_value;         
    unsigned char pad9C[0x06];
    short turn_rate;              
    short settle_rate;            
    unsigned char padA6[0x03];
    unsigned char enabled;        
} FieldMoveTarget;
typedef struct FieldActorNode {
      unsigned char pad_00[0x08];
      unsigned short flags;    
      unsigned char pad_0A[0x1A];
      struct FieldActorNode *next;
} FieldActorNode;
void Entity_WriteFieldByCmd(FieldActor *actor, int command, int value);
typedef struct BattleRewardSlot {
    s16 id;
    s16 extra;
} BattleRewardSlot;
extern u8 *g_CurItemEffectData asm("D_8009D1F8");
typedef union BattleParameterWord {
    u32 raw;
    struct {
        unsigned int first : 10;
        unsigned int second : 10;
        unsigned int third : 8;
        unsigned int reserved : 4;
    } fields;
} BattleParameterWord;
typedef struct BattleAttributes {
  BattleParameterWord parameterWord;
  u32 effectFlags;    
} BattleAttributes;
typedef struct EnemyActionEffect {
  u8  state;        
  u8  effectType;   
  u8  enterMode;
  u8  exitMode;
  s32 enterStep;
  s32 exitStep;
  u16 power;
  u8  category;     
  u8  frame;        
} EnemyActionEffect;
typedef struct Combatant {
  u32  coreFlags;      
  union {
               s16 fieldId04;   
               struct {
                   u8 rank;     
                   u8 field05;
               } bytes;
           } field04;
  s16  fieldId06;      
  s32  exp_or_acc;     
  u16  curHP;          
  u16  hpMirror;       
  u16  hpAlive;        
  u8   actionMode12;   
  s8   actionMode13;
  u8   actionMode14;
  u8   actionMode15;
  u8   actionMode16;
  u8   actionMode17;
  s8   actionMode18;
  s8   actionMode19;
  u8   pad_1A[2];
  u16  maxHP;          
  s16  stat1E;         
  s16  stat20;         
  u16  stat22;         
  u16  scaledOffense;  
  s16  stat26;         
  s32  maxAtk;         
  s32  atbStep;        
  s32  atbRate;        
  s32  atbGauge;       
  u16  subActionStep;
  u8   subActionCounter;
  u8   subActionPeriod;
  u16  statusStep3C;   
  u16  statusStep3E;   
  u16  statusTimer40;
  u16  statusTimer42;
  u16  statusTimer44;
  u16  statusTimer46;
  s8   knockbackFrames;
  u8   knockbackDistance;
  s16  knockbackAngle;
  s32  stateFlags;     
  s16  panelA_val;     
  u16  panelA_x;       
  u16  panelA_y;       
  u8   panelA_timer;   
  u8   panelA_scale;
  s16  panelB_val;     
  u16  panelB_x;
  u16  panelB_y;
  u8   panelB_timer;
  s8   panelB_flag;
  u16  panelAux_val;
  u16  panelAux_x;
  u16  panelAux_y;
  u8   panelAux_timer;  
  u8   panelAux_mode;
  struct BattleAction *action;  
  BattleAttributes *attributes;
  void *field70;       
  u8   pad_74[0x14];
  s32  field88;        
  u16  field8C;        
  u8   pad_8E[0x3E];
  s32  statusFlags2;   
  s16  panelC_val;     
  u16  panelC_x;
  u16  panelC_y;
  u8   panelC_timer;
} Combatant;
typedef struct EnemyCombatant {
  u32 coreFlags;
  union {
               s16 fieldId04;
               struct { u8 rank; u8 field05; } bytes;
           } field04;
  union {
               s16 field06;
               struct { u8 low; u8 entityId; } bytes;
           } field06;
  s32 field08;
  u16 curHP;
  u16 hpMirror;
  s32 hpAlive;
  u8  pad_14[4];
  EnemyActionEffect *effect;
  u8  pad_1C[0x30];
  u32 stateFlags;
  u8  pad_50[0x1C];
  BattleAttributes *attributes;
  u8  pad_70[0x18];
  s32 field88;
  u16 field8C;
  u8  pad_8E[2];
  u8  effectChance;
  u8  deathAnimFrame;
  u8  pad_92[2];
  u8  effectLevel;
  u8  effectDuration;
  u8  pad_96[2];
  u16 rewardBase;
  u16 rewardFactorA;
  u16 rewardFactorB;
  u8  rewardSlotId;
  u8  pad_9F;
  s16 lootItemId;
  s16 lootItemAux;
  u8  pad_A4[8];
  u8  deathAnimPhase;
  u8  deathFadeStep;
  u8  deathAssetEnabled;
  u8  deathPersist;
  u8  pad_B0[4];
  u16 deathAssetId;
  u8  pad_B6[0x16];
  u32 statusFlags2;
  u8  pad_D0[8];
} EnemyCombatant;
typedef struct BattleAction {
  u16  field00;
  s16  range;          
  u16  field04;
  union {
               s16 actionId;    
               struct { u8 actionIdByte; u8 field07; } bytes;
           } actionCode;
  s32  field08;        
  u32  attackWord;     
  u32  turnWord;       
  u8   animMode[4];    
} BattleAction;
typedef struct BattleEntity {
  void *core;  
  struct BattleEntity *next;  
  u8   pad_008[4];
  u8   modelIndex;      
  u8   teamId;         
  u8   actionMode;      
  u8   animLastFrame;   
  u8   pad_010[2];
  u16  animStopFrame;   
  s32  animFrame;       
  union {
               s32 fixed;  
               struct { u16 fraction; u16 integer; } parts;
           } animPrev;
  s32  animStep;        
  s32  moveFactor;
  u16  fieldSfxId;
  u16  moveSpeed;
  union { s32 fixed; struct { u16 frac; s16 integer; } parts; } posX;
  union { s32 fixed; struct { u16 frac; s16 integer; } parts; } posY;
  union { s32 fixed; struct { u16 frac; s16 integer; } parts; } posZ;
  u8   pad_034[4];
  s16  rotationX;
  s16  facingAngle;    
  s16  rotationZ;
  u8   pad_03E[2];
  s32  baseX;
  s32  baseY;
  s32  baseZ;
  u8   pad_04C[0x1C];
  s32  motionX;
  s32  motionY;
  s32  motionZ;
  u8   pad_074[0x24];
  u32  entityFlags;    
  u8  *scriptBase;
  u8   pad_0A0[0xEC];
  struct BattleEntity *parent;  
  u8   pad_190[4];
  void *actionCheckFn;  
  u8   pad_198[4];
  s32  scriptCursor19C;
  s32  scriptCursor1A0;
  void *collisionFace;
  void *collisionFaceMirror;
  s32  allocationActive;  
  void *actionData;     
  RenderObjectEntity renderObject;
  u8   pad_270[0x08];
  s32  allocationBlock;
} BattleEntity;
typedef struct BattleTarget {
  struct BattleEntity *actor;  
  s32  dist;                   
  s16  angle;                  
  u8   pad_0A[2];
} BattleTarget;                         
typedef struct BattleInitSlot {
    BattleEntity *actor;
    s16 field04;
    s16 field06;
} BattleInitSlot;
typedef char pe1_static_assert_battle_init_slot_size [( sizeof(BattleInitSlot) == 8 ) ? 1 : -1] ;
extern BattleTarget D_8009E000[];
void func_80021850(char *records, int from, int to);
void func_800216E4(char *records, s8 first, s8 last);
typedef char pe1_static_assert_combatant_field04_offset [( ((u32)&((( Combatant  *)0)->  field04 ))  == 0x04 ) ? 1 : -1] ;
typedef char pe1_static_assert_combatant_cur_hp_offset [( ((u32)&((( Combatant  *)0)->  curHP ))  == 0x0C ) ? 1 : -1] ;
typedef char pe1_static_assert_combatant_action_mode_offset [( ((u32)&((( Combatant  *)0)->  actionMode12 ))  == 0x12 ) ? 1 : -1] ;
typedef char pe1_static_assert_combatant_atb_step_offset [( ((u32)&((( Combatant  *)0)->  atbStep ))  == 0x2C ) ? 1 : -1] ;
typedef char pe1_static_assert_combatant_knockback_offset [( ((u32)&((( Combatant  *)0)->  knockbackFrames ))  == 0x48 ) ? 1 : -1] ;
typedef char pe1_static_assert_combatant_sub_action_step_offset [( ((u32)&((( Combatant  *)0)->  subActionStep ))  == 0x38 ) ? 1 : -1] ;
typedef char pe1_static_assert_combatant_status_timer_offset [( ((u32)&((( Combatant  *)0)->  statusStep3C ))  == 0x3C ) ? 1 : -1] ;
typedef char pe1_static_assert_combatant_aux_panel_offset [( ((u32)&((( Combatant  *)0)->  panelAux_val ))  == 0x60 ) ? 1 : -1] ;
typedef char pe1_static_assert_combatant_action_offset [( ((u32)&((( Combatant  *)0)->  action ))  == 0x68 ) ? 1 : -1] ;
typedef char pe1_static_assert_combatant_attributes_offset [( ((u32)&((( Combatant  *)0)->  attributes ))  == 0x6C ) ? 1 : -1] ;
typedef char pe1_static_assert_combatant_status_flags2_offset [( ((u32)&((( Combatant  *)0)->  statusFlags2 ))  == 0xCC ) ? 1 : -1] ;
typedef char pe1_static_assert_combatant_partial_size [( sizeof(Combatant) == 0xD8 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_action_partial_size [( sizeof(BattleAction) == 0x18 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_action_turn_word_offset [( ((u32)&((( BattleAction  *)0)->  turnWord ))  == 0x10 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_parameter_word_size [( sizeof(BattleParameterWord) == 4 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_attributes_size [( sizeof(BattleAttributes) == 0x08 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_attributes_effect_flags_offset [( ((u32)&((( BattleAttributes  *)0)->  effectFlags ))  == 0x04 ) ? 1 : -1] ;
typedef char pe1_static_assert_enemy_action_effect_size [( sizeof(EnemyActionEffect) == 0x10 ) ? 1 : -1] ;
typedef char pe1_static_assert_enemy_action_effect_power_offset [( ((u32)&((( EnemyActionEffect  *)0)->  power ))  == 0x0C ) ? 1 : -1] ;
typedef char pe1_static_assert_enemy_combatant_size [( sizeof(EnemyCombatant) == 0xD8 ) ? 1 : -1] ;
typedef char pe1_static_assert_enemy_combatant_effect_offset [( ((u32)&((( EnemyCombatant  *)0)->  effect ))  == 0x18 ) ? 1 : -1] ;
typedef char pe1_static_assert_enemy_combatant_attributes_offset [( ((u32)&((( EnemyCombatant  *)0)->  attributes ))  == 0x6C ) ? 1 : -1] ;
typedef char pe1_static_assert_enemy_combatant_effect_chance_offset [( ((u32)&((( EnemyCombatant  *)0)->  effectChance ))  == 0x90 ) ? 1 : -1] ;
typedef char pe1_static_assert_enemy_combatant_reward_base_offset [( ((u32)&((( EnemyCombatant  *)0)->  rewardBase ))  == 0x98 ) ? 1 : -1] ;
typedef char pe1_static_assert_enemy_combatant_loot_item_offset [( ((u32)&((( EnemyCombatant  *)0)->  lootItemId ))  == 0xA0 ) ? 1 : -1] ;
typedef char pe1_static_assert_enemy_combatant_death_phase_offset [( ((u32)&((( EnemyCombatant  *)0)->  deathAnimPhase ))  == 0xAC ) ? 1 : -1] ;
typedef char pe1_static_assert_enemy_combatant_death_asset_offset [( ((u32)&((( EnemyCombatant  *)0)->  deathAssetId ))  == 0xB4 ) ? 1 : -1] ;
typedef char pe1_static_assert_enemy_combatant_status_flags_offset [( ((u32)&((( EnemyCombatant  *)0)->  statusFlags2 ))  == 0xCC ) ? 1 : -1] ;
typedef char pe1_static_assert_combatant_views_hp_match [( ((u32)&((( EnemyCombatant  *)0)->  curHP ))  ==
                  ((u32)&((( Combatant  *)0)->  curHP ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_combatant_views_attributes_match [( ((u32)&((( EnemyCombatant  *)0)->  attributes ))  ==
                  ((u32)&((( Combatant  *)0)->  attributes ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_state_views_hp_match [( ((u32)&((( Combatant  *)0)->  curHP ))  ==
                  ((u32)&((( FieldActorState  *)0)->  amount ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_state_views_step_match [( ((u32)&((( Combatant  *)0)->  atbStep ))  ==
                  ((u32)&((( FieldActorState  *)0)->  progress_step ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_state_views_flags_match [( ((u32)&((( Combatant  *)0)->  stateFlags ))  ==
                  ((u32)&((( FieldActorState  *)0)->  flags ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_state_views_substate_match [( ((u32)&((( Combatant  *)0)->  attributes ))  ==
                  ((u32)&((( FieldActorState  *)0)->  substate ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_state_views_size_match [( sizeof(Combatant) == sizeof(FieldActorState) ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_pos_x_offset [( ((u32)&((( BattleEntity  *)0)->  posX ))  == 0x28 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_anim_frame_offset [( ((u32)&((( BattleEntity  *)0)->  animFrame ))  == 0x14 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_anim_prev_offset [( ((u32)&((( BattleEntity  *)0)->  animPrev ))  == 0x18 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_parent_offset [( ((u32)&((( BattleEntity  *)0)->  parent ))  == 0x18C ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_action_data_offset [( ((u32)&((( BattleEntity  *)0)->  actionData ))  == 0x1B0 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_collision_face_offset [( ((u32)&((( BattleEntity  *)0)->  collisionFace ))  == 0x1A4 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_pos_z_offset [( ((u32)&((( BattleEntity  *)0)->  posZ ))  == 0x30 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_facing_angle_offset [( ((u32)&((( BattleEntity  *)0)->  facingAngle ))  == 0x3A ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_motion_x_offset [( ((u32)&((( BattleEntity  *)0)->  motionX ))  == 0x68 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_motion_z_offset [( ((u32)&((( BattleEntity  *)0)->  motionZ ))  == 0x70 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_flags_offset [( ((u32)&((( BattleEntity  *)0)->  entityFlags ))  == 0x98 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_script_param_offset [( ((u32)&((( BattleEntity  *)0)->  renderObject.script_param97 ))  == 0x24B ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_render_flags_offset [( ((u32)&((( BattleEntity  *)0)->  renderObject.flags_9C ))  == 0x250 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_render_object_offset [( ((u32)&((( BattleEntity  *)0)->  renderObject ))  == 0x1B4 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_target_x_offset [( ((u32)&((( BattleEntity  *)0)->  renderObject.target_x ))  == 0x268 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_target_z_offset [( ((u32)&((( BattleEntity  *)0)->  renderObject.target_z ))  == 0x26C ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_allocation_block_offset [( ((u32)&((( BattleEntity  *)0)->  allocationBlock ))  == 0x278 ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_entity_size [( sizeof(BattleEntity) == 0x27C ) ? 1 : -1] ;
typedef char pe1_static_assert_battle_target_size [( sizeof(BattleTarget) == 0x0C ) ? 1 : -1] ;
typedef char pe1_static_assert_entity_views_next_match [( ((u32)&((( BattleEntity  *)0)->  next ))  ==
                  ((u32)&((( FieldActor  *)0)->  next ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_entity_views_anim_frame_match [( ((u32)&((( BattleEntity  *)0)->  animFrame ))  ==
                  ((u32)&((( FieldActor  *)0)->  anim ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_entity_views_pos_match [( ((u32)&((( BattleEntity  *)0)->  posX ))  ==
                  ((u32)&((( FieldActor  *)0)->  pos_x ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_entity_views_move_factor_match [( ((u32)&((( BattleEntity  *)0)->  moveFactor ))  ==
                  ((u32)&((( FieldActor  *)0)->  move_factor ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_entity_views_rotation_match [( ((u32)&((( BattleEntity  *)0)->  facingAngle ))  ==
                  ((u32)&((( FieldActor  *)0)->  rot_y ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_entity_views_flags_match [( ((u32)&((( BattleEntity  *)0)->  entityFlags ))  ==
                  ((u32)&((( FieldActor  *)0)->  flags ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_entity_views_script_base_match [( ((u32)&((( BattleEntity  *)0)->  scriptBase ))  ==
                  ((u32)&((( FieldActor  *)0)->  script_base ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_entity_views_parent_match [( ((u32)&((( BattleEntity  *)0)->  parent ))  ==
                  ((u32)&((( FieldActor  *)0)->  parent ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_entity_views_script_cursor_match [( ((u32)&((( BattleEntity  *)0)->  scriptCursor19C ))  ==
                  ((u32)&((( FieldActor  *)0)->  script_cursor_19c ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_entity_views_render_match [( ((u32)&((( BattleEntity  *)0)->  renderObject ))  ==
                  ((u32)&((( FieldActor  *)0)->  render_object ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_entity_views_render_field_match [( ((u32)&((( BattleEntity  *)0)->  renderObject.table_index ))  ==
                  ((u32)&((( FieldActor  *)0)->  render_object.table_index ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_entity_views_tail_match [( ((u32)&((( BattleEntity  *)0)->  allocationBlock ))  ==
                  ((u32)&((( FieldActor  *)0)->  allocation_block ))  ) ? 1 : -1] ;
typedef char pe1_static_assert_entity_views_size_match [( sizeof(BattleEntity) == sizeof(FieldActor) ) ? 1 : -1] ;
int func_800255E4(void);
void func_80023E14(int action);
extern u16 D_800BE9A0;
extern u8 D_800BE9A6, D_800BE9A7;
extern u16 D_800BD020, D_800BD022;
extern unsigned int D_8009D26C;
extern u32 D_8009D1A0;
void func_80071754(BattleEntity *entity);
void func_8007136C(BattleEntity *entity, int *actionState);
void func_800710A4(BattleEntity *entity, int *actionState);
int func_8003708C(int lhs, int rhs);
typedef struct PmSlotHeader {
    u8 state;
    u8 command;
    u8 field02;
    u8 field03;
    u32 ticks;
    void *owner;
} PmSlotHeader;
typedef int (*PmSendCallback)(PmSlotHeader *, int, int, int *, int *, int *);
typedef struct PmCommand {
    void *reserved;
    void (*initialize)(PmSlotHeader *);
    PmSendCallback send;
    int (*start)(void);
    int (*execute)(PmSlotHeader *);
    int (*stop)(void);
} PmCommand;
typedef char pe1_static_assert_pm_command_initializer_offset [( ((u32)&((( PmCommand  *)0)->  initialize ))  == 4 ) ? 1 : -1] ;
typedef char pe1_static_assert_pm_command_send_offset [( ((u32)&((( PmCommand  *)0)->  send ))  == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_pm_command_start_offset [( ((u32)&((( PmCommand  *)0)->  start ))  == 12 ) ? 1 : -1] ;
typedef char pe1_static_assert_pm_command_execute_offset [( ((u32)&((( PmCommand  *)0)->  execute ))  == 16 ) ? 1 : -1] ;
typedef char pe1_static_assert_pm_command_stop_offset [( ((u32)&((( PmCommand  *)0)->  stop ))  == 20 ) ? 1 : -1] ;
int func_8006F39C(u32 command, void *owner);
typedef struct PmPrimarySlot {
    PmSlotHeader header;
    u8 reserved0C[0xA00];
} PmPrimarySlot;
typedef struct PmSecondarySlot {
    PmSlotHeader header;
    u8 reserved0C[0x100];
} PmSecondarySlot;
typedef struct PmSlotBanks {
    PmPrimarySlot primary[11];
    PmSecondarySlot secondary[11];
} PmSlotBanks;
typedef char pe1_static_assert_pm_slot_header_size [( sizeof(PmSlotHeader) == 0x0C ) ? 1 : -1] ;
typedef char pe1_static_assert_pm_slot_ticks_offset [( ((u32)&((( PmSlotHeader  *)0)->  ticks ))  == 4 ) ? 1 : -1] ;
typedef char pe1_static_assert_pm_slot_owner_offset [( ((u32)&((( PmSlotHeader  *)0)->  owner ))  == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_pm_primary_slot_size [( sizeof(PmPrimarySlot) == 0xA0C ) ? 1 : -1] ;
typedef char pe1_static_assert_pm_secondary_slot_size [( sizeof(PmSecondarySlot) == 0x10C ) ? 1 : -1] ;
typedef char pe1_static_assert_pm_secondary_bank_offset [( ((u32)&((( PmSlotBanks  *)0)->  secondary ))  == 0x6E84 ) ? 1 : -1] ;
typedef char pe1_static_assert_pm_slot_banks_size [( sizeof(PmSlotBanks) == 0x7A08 ) ? 1 : -1] ;
extern PmPrimarySlot *D_800942E4;
extern PmSecondarySlot *D_800942E8;
extern u32 D_800E0EF0[];
int func_8006FC18(int slot, void *owner, int mode);
int func_8006FE14(void *owner);
extern Combatant *D_8009D278;
extern BattleEntity *D_8009D254;
extern s16 D_8009D2A4;
extern int D_8009D200;
extern u8 D_8009D294;
typedef struct BattleEvasionOutcome {
    s8 status;    
    s8 reaction;  
} BattleEvasionOutcome;
typedef char pe1_static_assert_battle_evasion_outcome_size [( sizeof(BattleEvasionOutcome) == 2 ) ? 1 : -1] ;
typedef struct BattleEnemySlot {
    s32 active;
    EnemyCombatant combatant;
} BattleEnemySlot;
extern BattleInitSlot D_800BE830[45];
extern int D_8009D2FC;
extern int D_8009D258;
extern int D_8009D208;
extern u8 D_8009CE44;
extern u8 D_8009CE40;
extern u8 D_8009D2D8;
extern u8 D_8009D1DC;
extern u8 D_8009CE38[4];
extern unsigned int D_8009D1AC;
extern u8 D_8009D1CE;
extern u8 D_8009CE60;
extern u8 D_8009D1D4;
extern u8 D_8009CE3C;
extern unsigned int D_8009D2E8;
extern int D_8009D28C;
extern BattleAction D_800A76D8;
extern s8 D_8009D2B0;
extern BattleEntity *D_8009D20C;
extern void *D_8009D1F8;
extern u8 D_8009159C[];
extern u8 D_800915C0[];
extern const EnemyCombatant D_800109B0;
extern BattleEnemySlot D_800A5D58[7];
extern u8 D_8009D2EC;
extern u8 D_8009D2A0;
extern u8 D_8009CE74;
extern u8 D_8009D244;
void func_80023008(void);
void func_80020F18(void);
void func_80021DE0(void);
void func_80070064(void);
int func_8006F6D4(int command, int arg1, int arg2, int arg3, int arg4, int arg5);
int func_8006DD38(int arg0, int arg1, int x, int y, int z);
int func_8006DE80(int id, int arg1, int x, int y, int z);
void func_800374E8(void);
extern s8 g_BattleTargetIndex asm("D_8009CE44");
extern s8 g_BattleActiveTurnSlot, D_8009D1F0 asm("D_8009D2D8");
extern unsigned int g_GameStateFlags asm("D_8009D1A0");
extern u8 D_8009D288;
extern unsigned int D_8009D1F4;
void func_80025BD8(BattleEntity *actor);
void func_80021128(void);
void func_800258CC(s8 mode);
void func_80021AF8(void);
void func_800275CC(BattleTarget *targets, int index);
void func_80026FF8(BattleTarget *targets, int index, int mode);
void func_800314E4(RenderObjectEntity *object, int mode, int outOfRange, int entityId);
void func_80026FD0(void);
void func_80021F38(void);
void func_80022210(void);
int func_80030534(BattleEntity *actor, BattleEntity *player);
void func_80021278(BattleEntity *actor, BattleEvasionOutcome *outcome,
                         int attackType);
void func_80022394(void);
void func_8005112C(void);
void func_8001A680(BattleEntity *entity, int mode);
void func_8001A784(BattleEntity *entity, int mode);
int func_800255E4(void);
void func_800218D8(void);
int func_80025EE8(void);
int func_80026824(s8 mode);
void func_80026600(BattleTarget *target);
void func_80026CF0(void);
void func_8002F7D8(BattleEntity *entity);
void func_8002F0B0(void);
void func_8003C5D8(RenderObjectEntity *object, int rate);
void func_800703F4(void);
int func_8006D60C(int mode);
void func_8002F9CC(void);
void func_800295E4(void);
void func_800293F4(int mode);
void func_800209F0(void);
void func_8006C4C4(int day);
void func_80022D7C(BattleEntity *entity);
void func_800325DC(void);
int func_80056C14(unsigned int category);
int func_80021054(void);
typedef char pe1_static_assert_battle_enemy_slot_size [( sizeof(BattleEnemySlot) == 0xDC ) ? 1 : -1] ;
int func_80066CE8(void)
{
    GteMatrixStorage rotation;
    GteShortVector axis;
    GteVector up,right,newUp,forward;
    int angle,cosine,sine;
    if((D_800BE9A0&0xF000)==0x7000)angle = D_800BD022;
    else angle = D_800BD020;
    {
        int masked = angle&4095;
        if(D_8009D254 && (D_8009D2E8&16)) {
            angle+=2048;
            angle+=(((unsigned int)((Combatant *)D_8009D254->core)->stateFlags)>>7)&0xC00;
            masked = angle&4095;
        }
        cosine = func_80077DC4(masked);
        sine = func_80077CF4(masked);
    }
    rotation.matrix.m[0][2] = sine;
    rotation.matrix.m[2][0] = -sine;
    rotation.matrix.m[1][1] = 4096;
    axis.z = 4096;
    asm volatile("":::"memory");
    rotation.matrix.m[0][0] = cosine;
    rotation.matrix.m[2][2] = cosine;
    rotation.matrix.t[0] = rotation.matrix.t[1] = rotation.matrix.t[2] = 0;
    rotation.matrix.m[0][1] = rotation.matrix.m[1][0] = rotation.matrix.m[1][2] = rotation.matrix.m[2][1] = 0;
    axis.x = 0;
    axis.y = 0;
    {
        register u32 *matrix asm("$2") = rotation.words;
        register u32 a asm("$12");
        register u32 b asm("$13");
        register u32 c asm("$14");
        asm("":"=r"(matrix):"0"(matrix):"memory");
        a = matrix[0];
        b = matrix[1];
        asm(""::"r"(a),"r"(b));
        asm volatile("ctc2 %0,$0" : : "r"( a )) ;
        asm volatile("ctc2 %0,$1" : : "r"( b )) ;
        a = matrix[2];
        b = matrix[3];
        c = matrix[4];
        asm(""::"r"(a),"r"(b),"r"(c));
        asm volatile("ctc2 %0,$2" : : "r"( a )) ;
        asm volatile("ctc2 %0,$3" : : "r"( b )) ;
        asm volatile("ctc2 %0,$4" : : "r"( c )) ;
        a = matrix[5];
        b = matrix[6];
        asm(""::"r"(a),"r"(b));
        asm volatile("ctc2 %0,$5" : : "r"( a )) ;
        c = matrix[7];
        asm volatile("ctc2 %0,$6" : : "r"( b )) ;
        asm volatile("ctc2 %0,$7" : : "r"( c )) ;
    }
    asm volatile("lwc2 $0,0(%0)"  : : "r"( &axis ) : "memory") ;
    asm volatile("lwc2 $1,4(%0)"  : : "r"( &axis ) : "memory") ;
    asm volatile("nop") ;
    asm volatile("nop") ;
    asm volatile(".word 0x4A480012") ;
    asm volatile("swc2 $25,0(%0)" : : "r"( &forward ) : "memory") ;
    asm volatile("swc2 $26,4(%0)" : : "r"( &forward ) : "memory") ;
    asm volatile("swc2 $27,8(%0)" : : "r"( &forward ) : "memory") ;
    {
        register GteVector *source asm("$2");
        register u32 a asm("$12");
        register u32 b asm("$13");
        register u32 c asm("$14");
        up.y = 4096;
        asm volatile("":::"memory");
        source = &up;
        asm(""::"r"(source):"memory");
        up.x = 0;
        up.z = 0;
        asm("":"=r"(source):"0"(source):"memory");
        a = source->x;
        b = source->y;
        asm(""::"r"(a),"r"(b));
        asm volatile("ctc2 %0,$0" : : "r"( a )) ;
        c = source->z;
        asm volatile("ctc2 %0,$2" : : "r"( b )) ;
        asm volatile("ctc2 %0,$4" : : "r"( c )) ;
    }
    {
        asm volatile("lwc2 $11,8(%0)" : : "r"( &forward ) : "memory") ;
        asm volatile("lwc2 $9,0(%0)" : : "r"( &forward ) : "memory") ;
        asm volatile("lwc2 $10,4(%0)" : : "r"( &forward ) : "memory") ;
        asm volatile("nop") ;
        asm volatile("nop") ;
        asm volatile(".word 0x4B78000C") ;
        asm volatile("swc2 $25,0(%0)" : : "r"( &right ) : "memory") ;
        asm volatile("swc2 $26,4(%0)" : : "r"( &right ) : "memory") ;
        asm volatile("swc2 $27,8(%0)" : : "r"( &right ) : "memory") ;
    }
    {
        register GteVector *source asm("$3") = &forward;
        register u32 a asm("$12");
        register u32 b asm("$13");
        register u32 c asm("$14");
        asm("":"=r"(source):"0"(source):"memory");
        a = source->x;
        b = source->y;
        asm(""::"r"(a),"r"(b));
        asm volatile("ctc2 %0,$0" : : "r"( a )) ;
        c = source->z;
        asm volatile("ctc2 %0,$2" : : "r"( b )) ;
        asm volatile("ctc2 %0,$4" : : "r"( c )) ;
    }
    {
        asm volatile("lwc2 $11,8(%0)" : : "r"( &right ) : "memory") ;
        asm volatile("lwc2 $9,0(%0)" : : "r"( &right ) : "memory") ;
        asm volatile("lwc2 $10,4(%0)" : : "r"( &right ) : "memory") ;
        asm volatile("nop") ;
        asm volatile("nop") ;
        asm volatile(".word 0x4B78000C") ;
        asm volatile("swc2 $25,0(%0)" : : "r"( &newUp ) : "memory") ;
        asm volatile("swc2 $26,4(%0)" : : "r"( &newUp ) : "memory") ;
        asm volatile("swc2 $27,8(%0)" : : "r"( &newUp ) : "memory") ;
    }
    {
        int xx = right.x,xy = newUp.x,xz = forward.x;
        int yx = right.y,yy = newUp.y,yz = forward.y;
        int zx = right.z,zy = newUp.z,zz = forward.z;
        D_800BD014 = 0;
        D_800BD018 = 0;
        D_800BD01C = 0;
        D_800BD000.m[0][0] = xx;
        D_800BD002 = xy;
        D_800BD004 = xz;
        D_800BD006 = yx;
        D_800BD008 = yy;
        D_800BD00A = yz;
        D_800BD00C = zx;
        D_800BD00E = zy;
        D_800BD010 = zz;
    }
    return 0;
}
