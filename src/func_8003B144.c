/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/main/render/Render_DrawTexturedQuads.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `Render_DrawTexturedQuads` renamed to `func_8003B144`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed int s32;
typedef unsigned int u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef float f32;
int func_80077DC4(int angle);
int func_80077CF4(int angle);
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
typedef struct PrimEntry {             
    unsigned char pad_00[0x04];
    unsigned char r;                   
    unsigned char g;                   
    unsigned char b;                   
    unsigned char pad_07[0x09];
} PrimEntry;
typedef struct PrimObj {
    unsigned char pad_00[0x26];
    unsigned short count;              
    unsigned char pad_28[0x08];
    PrimEntry *entries;                
} PrimObj;
typedef struct RenderLinePacket {
    u32 tag;
    u8 r, g, b, code;
    s16 x0, y0, x1, y1;
} RenderLinePacket;
typedef struct RenderTexturedQuad {
    union { u32 word; struct { u8 address[3], length; } bytes; } tag;
    union { u32 word; struct { u8 r, g, b, code; } bytes; } color;
    u16 x0, y0; u8 u0, v0; u16 clut;
    u16 x1, y1; u8 u1, v1; u16 tpage;
    u16 x2, y2; u8 u2, v2; u16 pad2;
    u16 x3, y3; u8 u3, v3; u16 pad3;
} RenderTexturedQuad;
typedef char pe1_static_assert_render_textured_quad_size [( sizeof(RenderTexturedQuad) == 40 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_quad_clut_offset [( ((u32)&((( RenderTexturedQuad  *)0)->  clut ))  == 14 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_quad_tpage_offset [( ((u32)&((( RenderTexturedQuad  *)0)->  tpage ))  == 22 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_quad_x3_offset [( ((u32)&((( RenderTexturedQuad  *)0)->  x3 ))  == 32 ) ? 1 : -1] ;
typedef struct RenderSpritePacket {
    union { u32 word; struct { u8 address[3], length; } bytes; } tag;
    union { u32 word; struct { u8 r, g, b, code; } bytes; } color;
    u16 x, y;
    u8 u, v;
    u16 clut, width, height;
} RenderSpritePacket;
typedef char pe1_static_assert_render_sprite_packet_size [( sizeof(RenderSpritePacket) == 20 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_sprite_x_offset [( ((u32)&((( RenderSpritePacket  *)0)->  x ))  == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_sprite_clut_offset [( ((u32)&((( RenderSpritePacket  *)0)->  clut ))  == 14 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_sprite_width_offset [( ((u32)&((( RenderSpritePacket  *)0)->  width ))  == 16 ) ? 1 : -1] ;
typedef struct RenderTilePacket {
    u8 address[3], length;
    u8 r, g, b, code;
    u16 x, y;
    u8 u, v;
    u16 clut;
} RenderTilePacket;
typedef struct RenderTexturePagePacket {
    u8 address[3], length;
    u32 command;
} RenderTexturePagePacket;
typedef union RenderTileTexture {
    u32 words[2];
    u8 bytes[8];
} RenderTileTexture;
typedef char pe1_static_assert_render_tile_packet_size [( sizeof(RenderTilePacket) == 16 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tile_clut_offset [( ((u32)&((( RenderTilePacket  *)0)->  clut ))  == 14 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_texture_page_packet_size [( sizeof(RenderTexturePagePacket) == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tile_texture_size [( sizeof(RenderTileTexture) == 8 ) ? 1 : -1] ;
extern u8 D_800BD024;
typedef struct RenderBufferPrefix {
    char *ordering[2];
    char *other_buffers[6];
    char *packets[2];
} RenderBufferPrefix;
typedef char pe1_static_assert_render_line_packet_size [( sizeof(RenderLinePacket) == 16 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_line_packet_points [( ((u32)&((( RenderLinePacket  *)0)->  x0 ))  == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_buffer_prefix_packets [( ((u32)&((( RenderBufferPrefix  *)0)->  packets ))  == 0x20 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_buffer_prefix_size [( sizeof(RenderBufferPrefix) == 0x28 ) ? 1 : -1] ;
extern RenderBufferPrefix D_800B0E38;
extern int D_8009CDD8, D_8009CDDC;
void func_80077C64(RenderLinePacket *packet);
u16 func_80077A64(int tp, int abr, int x, int y);
void func_80077C84(char *packet, int drawTexture, int dither, int tpage);
void func_800CF6F8(void *ordering, void *packet, int mode);
struct RenderColor;
void func_800DB25C(GteShortVector *position, int x, int y, int angle,
                   int radius, int depth, int intensity, struct RenderColor *color);
extern u32 D_800B1644[];
extern s32 D_800A636C[];
void func_8003B144(RenderObjectEntity *input) {
    u32 reserve[8];
    RenderObjectEntity *entity = input;
    int i = 0;
    u32 *xy;
    s32 *z;
    register RenderPrimitiveDescriptor *record asm("$15");
    register u8 *packets asm("$14");
    int slot;
    u32 *ordering;
    volatile s32 *area;
    asm("" : "=r"(area));
    asm("" : : "r"(area));
    xy = D_800B1644;
    z = D_800A636C;
    record = entity->primitive_descriptors;
    packets = entity->primitive_buffer;
    slot = D_8009CDDC;
    asm volatile("" : "=r"(slot) : "0"(slot) : "memory");
    {
        RenderObjectHeader *header = entity->header;
        int count;
        register int table_offset asm("$2");
        asm("" : "=r"(header) : "0"(header));
        table_offset = slot * 4;
        count = header->packet34_count;
        asm("" : "=r"(count) : "0"(count));
        ordering = *(u32 **)((u8 *)&D_800B0E38 + table_offset);
        asm volatile("" : "=r"(ordering) : "0"(ordering) : "$19");
        area = (volatile s32 *)0x1F800000;
        if (count > 0) {
            register int product asm("$2") = slot * 13;
            register int slot_offset asm("$22");
            register u32 address_mask asm("$24");
            register u32 length_mask asm("$20");
            register u16 *window asm("$10");
            asm("" : "=r"(product) : "0"(product));
            slot_offset = product * 4;
            address_mask = 0xFFFFFF;
            length_mask = 0xFF000000;
            window = &record->lookup_indices[3];
            do {
                register int a asm("$2") = window[-3];
                int b = window[-2];
                register int c asm("$4") = window[-1];
                int offset_a;
                int offset_c;
                u32 xy0;
                u32 xy1;
                u32 xy2;
                u8 *packet;
                u32 *coord;
                register int d asm("$5");
                asm("" : "=r"(a), "=r"(b), "=r"(c) : "0"(a), "1"(b), "2"(c));
                offset_a = a * 4;
                coord = (u32 *)((u8 *)((u32)(  offset_a ) + (u32)( xy ))) ;
                b *= 4;
                xy0 = *coord;
                coord = (u32 *)((u8 *)((u32)(  b ) + (u32)( xy ))) ;
                asm("" : "=r"(coord) : "0"(coord));
                offset_c = c * 4;
                xy1 = *coord;
                coord = (u32 *)((u8 *)((u32)(  offset_c ) + (u32)( xy ))) ;
                xy2 = *coord;
                asm volatile("mtc2 %0,$12" : : "r"( xy0 )) ;
                asm volatile("mtc2 %0,$14" : : "r"( xy2 )) ;
                asm volatile("mtc2 %0,$13" : : "r"( xy1 )) ;
                asm volatile("nop") ;
                asm volatile("nop") ;
                asm volatile(".word 0x4B400006") ;
                packet = packets + slot_offset;
                d = window[0];
                asm volatile("swc2 $24,0(%0)" : : "r"( area ) : "memory") ;
                if (*area > 0) {
                    register s32 depth asm("$4");
                    {
                        register s32 *p0 asm("$2") = (s32 *)((u8 *)((u32)(  offset_a ) + (u32)( z ))) ;
                        s32 *p1 = (s32 *)((u8 *)((u32)(  b ) + (u32)( z ))) ;
                        register s32 value0 asm("$2");
                        s32 value1;
                        asm("" : "=r"(p0), "=r"(p1) : "0"(p0), "1"(p1));
                        depth = *p0;
                        value0 = *p1;
                        p1 = (s32 *)((u8 *)((u32)(  offset_c ) + (u32)( z ))) ;
                        d *= 4;
                        value1 = *p1;
                        depth += value0;
                        p0 = (s32 *)((u8 *)((u32)(  d ) + (u32)( z ))) ;
                        value0 = *p0;
                        depth += value1;
                        depth += value0;
                    }
                    depth >>= 4;
                    if (depth < 0x1000) {
                        u32 *entry;
                        depth *= 4;
                        entry = (u32 *)((u8 *)((u32)(  depth ) + (u32)( ordering ))) ;
                        ((RenderPacket34 *)packet)->tag =
                            (((RenderPacket34 *)packet)->tag & length_mask) |
                            (*entry & address_mask);
                        *entry = (*entry & length_mask) | ((u32)packet & address_mask);
                        asm volatile("" : "=r"(d) : "0"(d) : "memory");
                        {
                            register u32 *last asm("$2") = (u32 *)((u8 *)((u32)(  d ) + (u32)( xy ))) ;
                            asm("" : "=r"(last) : "0"(last));
                            ((RenderPacket34 *)packet)->sxy0 = xy0;
                            ((RenderPacket34 *)packet)->sxy1 = xy1;
                            ((RenderPacket34 *)packet)->sxy2 = xy2;
                            ((RenderPacket34 *)packet)->sxy3 = *last;
                        }
                    }
                } else {
                    ((RenderPacket34 *)packet)->tag &= length_mask;
                }
                packets += 104;
                i++;
                window += 6;
                record++;
            } while (i < entity->header->packet34_count);
        }
    }
    {
        int count = entity->header->packet28_count;
        i = 0;
        if (count > 0) {
            register int product asm("$2") = slot * 5;
            register int slot_offset asm("$20");
            u32 address_mask;
            register u32 length_mask asm("$24");
            register u16 *window asm("$8");
            asm("" : "=r"(product) : "0"(product));
            slot_offset = product * 8;
            address_mask = 0xFFFFFF;
            length_mask = 0xFF000000;
            window = &record->lookup_indices[2];
            asm(""
                : "=r"(slot_offset), "=r"(address_mask), "=r"(length_mask), "=r"(window)
                : "0"(slot_offset), "1"(address_mask), "2"(length_mask), "3"(window));
            do {
                register int a asm("$2") = window[-2];
                int b = window[-1];
                int c = window[0];
                int offset_a;
                int offset_c;
                u32 xy0;
                u32 xy1;
                register u32 xy2 asm("$9");
                u8 *packet;
                register u32 *coord asm("$2");
                asm("" : "=r"(a), "=r"(b), "=r"(c) : "0"(a), "1"(b), "2"(c));
                offset_a = a * 4;
                coord = (u32 *)((u8 *)((u32)(  offset_a ) + (u32)( xy ))) ;
                asm("" : "=r"(coord) : "0"(coord));
                b *= 4;
                xy0 = *coord;
                coord = (u32 *)((u8 *)((u32)(  b ) + (u32)( xy ))) ;
                asm("" : "=r"(coord) : "0"(coord));
                offset_c = c * 4;
                xy1 = *coord;
                coord = (u32 *)((u8 *)((u32)(  offset_c ) + (u32)( xy ))) ;
                asm("" : "=r"(coord) : "0"(coord));
                xy2 = *coord;
                asm volatile("mtc2 %0,$12" : : "r"( xy0 )) ;
                asm volatile("mtc2 %0,$14" : : "r"( xy2 )) ;
                asm volatile("mtc2 %0,$13" : : "r"( xy1 )) ;
                asm volatile("nop") ;
                asm volatile("nop") ;
                asm volatile(".word 0x4B400006") ;
                packet = packets + slot_offset;
                asm volatile("swc2 $24,0(%0)" : : "r"( area ) : "memory") ;
                if (*area > 0) {
                    register s32 depth asm("$4");
                    {
                        s32 *p0 = (s32 *)((u8 *)((u32)(  offset_a ) + (u32)( z ))) ;
                        s32 *p1 = (s32 *)((u8 *)((u32)(  b ) + (u32)( z ))) ;
                        s32 *p2 = (s32 *)((u8 *)((u32)(  offset_c ) + (u32)( z ))) ;
                        register s32 value0 asm("$2");
                        s32 value1;
                        register s32 quotient asm("$2");
                        value0 = *p0;
                        value1 = *p1;
                        depth = *p2;
                        value0 += value1;
                        depth = value0 + depth;
                        quotient = depth / 3;
                        depth = quotient >> 2;
                    }
                    if (depth < 0x1000) {
                        u32 *entry;
                        depth *= 4;
                        entry = (u32 *)((u8 *)((u32)(  depth ) + (u32)( ordering ))) ;
                        ((RenderPacket28 *)packet)->tag =
                            (((RenderPacket28 *)packet)->tag & length_mask) |
                            (*entry & address_mask);
                        *entry = (*entry & length_mask) | ((u32)packet & address_mask);
                        ((RenderPacket28 *)packet)->sxy0 = xy0;
                        ((RenderPacket28 *)packet)->sxy1 = xy1;
                        ((RenderPacket28 *)packet)->sxy2 = xy2;
                    }
                } else {
                    ((RenderPacket28 *)packet)->tag &= length_mask;
                }
                packets += 80;
                i++;
                window += 6;
                record++;
            } while (i < entity->header->packet28_count);
        }
    }
    {
        int count = entity->header->packet24_count;
        i = 0;
        if (count > 0) {
            register int product asm("$2") = slot * 9;
            register int slot_offset asm("$22");
            register u32 address_mask asm("$24");
            register u32 length_mask asm("$20");
            register u16 *window asm("$10");
            asm("" : "=r"(product) : "0"(product));
            slot_offset = product * 4;
            address_mask = 0xFFFFFF;
            length_mask = 0xFF000000;
            window = &record->lookup_indices[3];
            do {
                register int a asm("$2") = window[-3];
                int b = window[-2];
                register int c asm("$4") = window[-1];
                int offset_a;
                int offset_c;
                u32 xy0;
                u32 xy1;
                u32 xy2;
                u8 *packet;
                u32 *coord;
                register int d asm("$5");
                asm("" : "=r"(a), "=r"(b), "=r"(c) : "0"(a), "1"(b), "2"(c));
                offset_a = a * 4;
                coord = (u32 *)((u8 *)((u32)(  offset_a ) + (u32)( xy ))) ;
                b *= 4;
                xy0 = *coord;
                coord = (u32 *)((u8 *)((u32)(  b ) + (u32)( xy ))) ;
                asm("" : "=r"(coord) : "0"(coord));
                offset_c = c * 4;
                xy1 = *coord;
                coord = (u32 *)((u8 *)((u32)(  offset_c ) + (u32)( xy ))) ;
                xy2 = *coord;
                asm volatile("mtc2 %0,$12" : : "r"( xy0 )) ;
                asm volatile("mtc2 %0,$14" : : "r"( xy2 )) ;
                asm volatile("mtc2 %0,$13" : : "r"( xy1 )) ;
                asm volatile("nop") ;
                asm volatile("nop") ;
                asm volatile(".word 0x4B400006") ;
                packet = packets + slot_offset;
                d = window[0];
                asm volatile("swc2 $24,0(%0)" : : "r"( area ) : "memory") ;
                if (*area > 0) {
                    register s32 depth asm("$4");
                    {
                        register s32 *p0 asm("$2") = (s32 *)((u8 *)((u32)(  offset_a ) + (u32)( z ))) ;
                        s32 *p1 = (s32 *)((u8 *)((u32)(  b ) + (u32)( z ))) ;
                        register s32 value0 asm("$2");
                        s32 value1;
                        asm("" : "=r"(p0), "=r"(p1) : "0"(p0), "1"(p1));
                        depth = *p0;
                        value0 = *p1;
                        p1 = (s32 *)((u8 *)((u32)(  offset_c ) + (u32)( z ))) ;
                        d *= 4;
                        value1 = *p1;
                        depth += value0;
                        p0 = (s32 *)((u8 *)((u32)(  d ) + (u32)( z ))) ;
                        value0 = *p0;
                        depth += value1;
                        depth += value0;
                    }
                    depth >>= 4;
                    if (depth < 0x1000) {
                        u32 *entry;
                        depth *= 4;
                        entry = (u32 *)((u8 *)((u32)(  depth ) + (u32)( ordering ))) ;
                        ((RenderPacket24 *)packet)->tag =
                            (((RenderPacket24 *)packet)->tag & length_mask) |
                            (*entry & address_mask);
                        *entry = (*entry & length_mask) | ((u32)packet & address_mask);
                        asm volatile("" : "=r"(d) : "0"(d) : "memory");
                        {
                            register u32 *last asm("$2") = (u32 *)((u8 *)((u32)(  d ) + (u32)( xy ))) ;
                            asm("" : "=r"(last) : "0"(last));
                            ((RenderPacket24 *)packet)->sxy0 = xy0;
                            ((RenderPacket24 *)packet)->sxy1 = xy1;
                            ((RenderPacket24 *)packet)->sxy2 = xy2;
                            ((RenderPacket24 *)packet)->sxy3 = *last;
                        }
                    }
                } else {
                    ((RenderPacket24 *)packet)->tag &= length_mask;
                }
                packets += 72;
                i++;
                window += 6;
                record++;
            } while (i < entity->header->packet24_count);
        }
    }
    {
        int count = entity->header->packet1c_count;
        i = 0;
        if (count > 0) {
            register int product asm("$2") = slot * 7;
            register int slot_offset asm("$20");
            u32 address_mask;
            register u32 length_mask asm("$18");
            register u16 *window asm("$8");
            asm("" : "=r"(product) : "0"(product));
            slot_offset = product * 4;
            address_mask = 0xFFFFFF;
            length_mask = 0xFF000000;
            window = &record->lookup_indices[2];
            do {
                register int a asm("$2") = window[-2];
                int b = window[-1];
                int c = window[0];
                int offset_a;
                int offset_c;
                register u32 xy0 asm("$12");
                register u32 xy1 asm("$11");
                register u32 xy2 asm("$9");
                u8 *packet;
                register u32 *coord asm("$2");
                asm("" : "=r"(a), "=r"(b), "=r"(c) : "0"(a), "1"(b), "2"(c));
                offset_a = a * 4;
                coord = (u32 *)((u8 *)((u32)(  offset_a ) + (u32)( xy ))) ;
                asm("" : "=r"(coord) : "0"(coord));
                b *= 4;
                xy0 = *coord;
                coord = (u32 *)((u8 *)((u32)(  b ) + (u32)( xy ))) ;
                asm("" : "=r"(coord) : "0"(coord));
                offset_c = c * 4;
                xy1 = *coord;
                coord = (u32 *)((u8 *)((u32)(  offset_c ) + (u32)( xy ))) ;
                asm("" : "=r"(coord) : "0"(coord));
                xy2 = *coord;
                asm volatile("mtc2 %0,$12" : : "r"( xy0 )) ;
                asm volatile("mtc2 %0,$14" : : "r"( xy2 )) ;
                asm volatile("mtc2 %0,$13" : : "r"( xy1 )) ;
                asm volatile("nop") ;
                asm volatile("nop") ;
                asm volatile(".word 0x4B400006") ;
                packet = packets + slot_offset;
                asm volatile("swc2 $24,0(%0)" : : "r"( area ) : "memory") ;
                if (*area > 0) {
                    register s32 depth asm("$4");
                    {
                        s32 *p0 = (s32 *)((u8 *)((u32)(  offset_a ) + (u32)( z ))) ;
                        s32 *p1 = (s32 *)((u8 *)((u32)(  b ) + (u32)( z ))) ;
                        s32 *p2 = (s32 *)((u8 *)((u32)(  offset_c ) + (u32)( z ))) ;
                        s32 value0;
                        s32 value1;
                        register s32 quotient asm("$2");
                        value0 = *p0;
                        value1 = *p1;
                        depth = *p2;
                        value0 += value1;
                        depth = value0 + depth;
                        quotient = depth / 3;
                        depth = quotient >> 2;
                    }
                    if (depth < 0x1000) {
                        u32 *entry;
                        depth *= 4;
                        entry = (u32 *)((u8 *)((u32)(  depth ) + (u32)( ordering ))) ;
                        ((RenderPacket1C *)packet)->tag =
                            (((RenderPacket1C *)packet)->tag & length_mask) |
                            (*entry & address_mask);
                        *entry = (*entry & length_mask) | ((u32)packet & address_mask);
                        ((RenderPacket1C *)packet)->sxy0 = xy0;
                        ((RenderPacket1C *)packet)->sxy1 = xy1;
                        ((RenderPacket1C *)packet)->sxy2 = xy2;
                    }
                } else {
                    ((RenderPacket1C *)packet)->tag &= length_mask;
                }
                packets += 56;
                i++;
                window += 6;
                record++;
            } while (i < entity->header->packet1c_count);
        }
    }
}
