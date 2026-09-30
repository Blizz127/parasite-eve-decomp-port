/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/scene_e08/Scene_InitParticleCluster_8018F958.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `Scene_InitParticleCluster_8018F958` renamed to `func_8018F958`, and vendor symbol
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
extern s32 D_8009D038;
extern u8 D_800A1B90[521];
int func_80071A54 (void);
int Engine_Random(void) __asm__("func_80071A54");
typedef struct RoomSpriteMatrix {
    short m[3][3];
    short pad;
    int t[3];
} RoomSpriteMatrix;
typedef struct RoomFxSeed8 {
    unsigned char bytes[8];
} RoomFxSeed8;
typedef struct RoomFxVec4 {
    int x;
    int y;
    int z;
    int w;
} RoomFxVec4;
typedef struct RoomUniformSpriteFxParams {
    short x;
    short y;
    short z;
    short pad6;
    short scale;
    unsigned short depth;
} RoomUniformSpriteFxParams;
typedef struct RoomOrbitParticlePosition {
    unsigned short x;
    unsigned short y;
    unsigned short z;
    unsigned short pad6;
} RoomOrbitParticlePosition;
typedef struct RoomOrbitParticleVelocity {
    short x;
    unsigned short angle;
    short z;
    short pad6;
} RoomOrbitParticleVelocity;
typedef struct RoomOrbitParticleLaneView {
    RoomOrbitParticlePosition position;
    unsigned char remainingPositions[7 * sizeof(RoomOrbitParticlePosition)];
    RoomOrbitParticleVelocity velocity;
} RoomOrbitParticleLaneView;
typedef struct RoomOrbitParticleState {
    RoomOrbitParticlePosition position[8];
    RoomOrbitParticleVelocity velocity[8];
    unsigned short height;
    short decay;
    unsigned char frame;
    unsigned char intensity;
    short radius;
    short radiusStep;
} RoomOrbitParticleState;
typedef struct RoomOrbitBurstVector {
    short x;
    short y;
    short z;
    short pad6;
} RoomOrbitBurstVector;
typedef struct RoomOrbitBurstState {
    RoomOrbitBurstVector position[8];
    RoomOrbitBurstVector velocity[8];
    RoomOrbitBurstVector secondary[8];
    unsigned char active[8];
    unsigned char frame[8];
    short scale;
    unsigned short depth;
    unsigned char count;
    unsigned char padD5;
    short phaseStep;
} RoomOrbitBurstState;
typedef char pe1_static_assert_room_orbit_burst_velocity_offset [( ((u32)&((( RoomOrbitBurstState  *)0)->  velocity ))  == 0x40 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_orbit_burst_secondary_offset [( ((u32)&((( RoomOrbitBurstState  *)0)->  secondary ))  == 0x80 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_orbit_burst_active_offset [( ((u32)&((( RoomOrbitBurstState  *)0)->  active ))  == 0xC0 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_orbit_burst_state_size [( sizeof(RoomOrbitBurstState) == 0xD8 ) ? 1 : -1] ;
typedef struct RoomSpriteFxParams {
    short x;
    short y;
    short z;
    unsigned char pad6[0xA];
    short scale;
    unsigned short depth;
    unsigned char pad14;
    unsigned char alpha;
} RoomSpriteFxParams;
typedef struct RoomOrbitSpriteFxParams {
    short x;
    short y;
    short z;
    unsigned char pad6[0xA];
    short scale;
    unsigned short depth;
    unsigned char pad14[2];
    unsigned short alpha;
} RoomOrbitSpriteFxParams;
typedef struct RoomSeededSpriteFxParams {
    short x;
    short y;
    short z;
    unsigned char pad6[0x2];
    RoomFxSeed8 seed;
    short scale;
    unsigned short depth;
} RoomSeededSpriteFxParams;
typedef struct RoomDoubleSpriteFxParams {
    short x;
    short y;
    short z;
    unsigned char pad6[2];
    RoomFxSeed8 seed;
    short scale;
    unsigned short depth;
    unsigned char alpha;
} RoomDoubleSpriteFxParams;
typedef struct RoomDoubleSpriteGlobals {
    unsigned char alpha;
    unsigned char pad1[5];
    unsigned short depth;
} RoomDoubleSpriteGlobals;
typedef char pe1_static_assert_room_double_sprite_seed_offset [( ((u32)&((( RoomDoubleSpriteFxParams  *)0)->  seed ))  == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_double_sprite_scale_offset [( ((u32)&((( RoomDoubleSpriteFxParams  *)0)->  scale ))  == 0x10 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_double_sprite_depth_offset [( ((u32)&((( RoomDoubleSpriteFxParams  *)0)->  depth ))  == 0x12 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_double_sprite_alpha_offset [( ((u32)&((( RoomDoubleSpriteFxParams  *)0)->  alpha ))  == 0x14 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_double_sprite_global_depth_offset [( ((u32)&((( RoomDoubleSpriteGlobals  *)0)->  depth ))  == 6 ) ? 1 : -1] ;
typedef struct RoomLayeredSpriteParams {
    short x;
    short y;
    short z;
    unsigned char pad6[10];
    short scale;
    unsigned short pad12;
    unsigned short depth;
} RoomLayeredSpriteParams;
typedef char pe1_static_assert_room_layered_sprite_scale_offset [( ((u32)&((( RoomLayeredSpriteParams  *)0)->  scale ))  == 0x10 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_layered_sprite_depth_offset [( ((u32)&((( RoomLayeredSpriteParams  *)0)->  depth ))  == 0x14 ) ? 1 : -1] ;
typedef struct RoomFxDriftParticle {
    short x;
    short y;
    short z;
    short pad6;
    int radius;
} RoomFxDriftParticle;
typedef struct RoomFxTrajectoryParticle {
    unsigned short x;
    unsigned short y;
    unsigned short z;
    unsigned short pad6;
} RoomFxTrajectoryParticle;
typedef struct RoomFxGroundSpriteParams {
    short x;
    short pad02;
    short z;
    unsigned char pad06[0x7C];
    unsigned short depth;
} RoomFxGroundSpriteParams;
typedef struct RoomFxTimedRenderState {
    unsigned char pad00[8];
    short disabled;
    unsigned short frame;
} RoomFxTimedRenderState;
typedef struct RoomFallingParticleControl {
    unsigned char pad00;
    unsigned char state;
} RoomFallingParticleControl;
typedef struct RoomFallingParticleState {
    unsigned char active;
    unsigned char pad01;
    unsigned char frame;
    unsigned char pad03;
    unsigned short phase;
    short intensity;
    unsigned short x;
    unsigned short y;
    unsigned short z;
    unsigned char pad0E[0xA];
    unsigned short velocityX;
    unsigned short velocityY;
    unsigned short velocityZ;
} RoomFallingParticleState;
typedef char pe1_static_assert_room_falling_particle_velocity_x_offset [( ((u32)&((( RoomFallingParticleState  *)0)->  velocityX ))  == 0x18 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_falling_particle_state_size [( sizeof(RoomFallingParticleState) == 0x1E ) ? 1 : -1] ;
typedef struct RoomEightParticleVector {
    unsigned short x;
    unsigned short y;
    unsigned short z;
    unsigned short pad06;
} RoomEightParticleVector;
typedef struct RoomEightParticleState {
    int activeCount;
    RoomEightParticleVector position[8];
    RoomFxSeed8 velocity[8];
    unsigned short scale[8];
    short fade[8];
    unsigned short angle[8];
    unsigned char active[8];
} RoomEightParticleState;
typedef struct RoomEightParticleControl {
    unsigned char pad00;
    unsigned char state;
    short frame;
} RoomEightParticleControl;
typedef struct RoomEightParticleContext {
    char *root;
    unsigned char pad04[0x34];
    int baseX;
    int baseY;
    int baseZ;
} RoomEightParticleContext;
typedef struct RoomFxTransform {
    int pad00[5];
    int x;
    int y;
    int z;
} RoomFxTransform;
typedef struct RoomFxTransformOwner {
    unsigned char pad00[0x238];
    RoomFxTransform *transforms;
} RoomFxTransformOwner;
typedef struct RoomFxControl {
    unsigned char pad00[2];
    short frame;
} RoomFxControl;
typedef struct RoomFxPairedSpriteState {
    unsigned short x;
    unsigned short y;
    unsigned short z;
    unsigned char pad06[2];
    short velocityX;
    short velocityY;
    short velocityZ;
    unsigned char pad0E[2];
    short alpha;
    unsigned short alphaStep;
    unsigned short sparkleTimer;
    unsigned char pad16[2];
    unsigned short sparkleX;
    unsigned short sparkleY;
    unsigned short sparkleZ;
    unsigned char pad1E[2];
    unsigned short sparkleAlpha;
    short sparkleLife;
    short resourceSelector;
    short active;
    short height;
    short width;
    short phase;
    short transformIndex;
    unsigned int counter;
    short minimumHeight;
} RoomFxPairedSpriteState;
typedef char pe1_static_assert_room_fx_transform_size [( sizeof(RoomFxTransform) == 0x20 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_fx_transform_owner_transforms_offset [( ((u32)&((( RoomFxTransformOwner  *)0)->  transforms ))  == 0x238 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_fx_paired_sprite_sparkle_x_offset [( ((u32)&((( RoomFxPairedSpriteState  *)0)->  sparkleX ))  == 0x18 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_fx_paired_sprite_resource_selector_offset [( ((u32)&((( RoomFxPairedSpriteState  *)0)->  resourceSelector ))  == 0x24 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_fx_paired_sprite_transform_index_offset [( ((u32)&((( RoomFxPairedSpriteState  *)0)->  transformIndex ))  == 0x2E ) ? 1 : -1] ;
typedef char pe1_static_assert_room_fx_paired_sprite_minimum_height_offset [( ((u32)&((( RoomFxPairedSpriteState  *)0)->  minimumHeight ))  == 0x34 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_fx_paired_sprite_state_size [( sizeof(RoomFxPairedSpriteState) == 0x38 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_eight_particle_vector_size [( sizeof(RoomEightParticleVector) == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_eight_particle_velocity_offset [( ((u32)&((( RoomEightParticleState  *)0)->  velocity ))  == 0x44 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_eight_particle_scale_offset [( ((u32)&((( RoomEightParticleState  *)0)->  scale ))  == 0x84 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_eight_particle_fade_offset [( ((u32)&((( RoomEightParticleState  *)0)->  fade ))  == 0x94 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_eight_particle_angle_offset [( ((u32)&((( RoomEightParticleState  *)0)->  angle ))  == 0xA4 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_eight_particle_active_offset [( ((u32)&((( RoomEightParticleState  *)0)->  active ))  == 0xB4 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_eight_particle_state_size [( sizeof(RoomEightParticleState) == 0xBC ) ? 1 : -1] ;
typedef char pe1_static_assert_room_eight_particle_context_base_x_offset [( ((u32)&((( RoomEightParticleContext  *)0)->  baseX ))  == 0x38 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_fx_ground_sprite_depth_offset [( ((u32)&((( RoomFxGroundSpriteParams  *)0)->  depth ))  == 0x82 ) ? 1 : -1] ;
typedef struct RoomFxDriftState {
    unsigned short x;
    short pad02;
    unsigned short z;
    short pad06;
    short dx;
    short pad0A;
    short dz;
    short pad0E;
    unsigned short phase10;
    short limit12;
    short phase14;
    unsigned short counter16;
} RoomFxDriftState;
typedef struct RoomFxEmitterParams {
    void *source;
    unsigned char color0[3];
    unsigned char pad07;
    unsigned char color1[3];
    unsigned char pad0B;
    short mode;
    short extent0;
    short extent1;
    short offset;
    short intensity;
    short pad16;
} RoomFxEmitterParams;
typedef struct RoomFxPairedEmitterState {
    unsigned char header[8];
    unsigned char sourceData[0x100];
    RoomFxEmitterParams primary;
    RoomFxEmitterParams secondary;
    short timer;
    short intensity;
    short phase;
} RoomFxPairedEmitterState;
typedef char pe1_static_assert_room_fx_emitter_params_size [( sizeof(RoomFxEmitterParams) == 0x18 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_fx_paired_primary_offset [( ((u32)&((( RoomFxPairedEmitterState  *)0)->  primary ))  == 0x108 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_fx_paired_secondary_offset [( ((u32)&((( RoomFxPairedEmitterState  *)0)->  secondary ))  == 0x120 ) ? 1 : -1] ;
typedef char pe1_static_assert_room_fx_paired_state_size [( sizeof(RoomFxPairedEmitterState) == 0x140 ) ? 1 : -1] ;
typedef struct {
    RoomFxEmitterParams emitters[2];
    char sourceData[2][0x100];
    s16 field_230[2];
    char pad_234[0x20];
    s16 field_254[4];
    s16 field_25C[4];
    s16 field_264;
} Ovl178FadeState;
typedef struct {
    char pad_00[0x18];
    s32 x;
    s32 y;
    s32 z;
} Ovl178Position;
typedef struct {
    s32 field_00;
    s32 field_04;
    s32 field_08;
    char pad_0C[4];
    s32 field_10;
    s32 field_14;
    s32 field_18;
    char pad_1C[4];
    s16 field_20;
    s16 field_22;
    s16 field_24;
    char pad_26[2];
    u8 field_28;
    char pad_29;
    s16 field_2A;
    s32 field_2C;
} Ovl178EffectState;
typedef struct {
    u8 pad_00[8];
    s16 field_08;
    s16 field_0A;
    s16 offsets[12];
    union {
        s16 variation[3];
        u8 flags[10];
    } tail;
} Ovl178RandomizedEffect;
Ovl178Position *func_800C2B50(void);
s32 *func_800C2B10(int index);
s32 *func_800C2B28(int index);
extern s16 D_80199590[];
extern s16 D_80199658[];
extern s32 D_8019956C;
extern s32 D_8019957C;
extern void *volatile D_800B0E64;
void func_8006DF50(void *sound, int cue, int arg2, int volume, int pan);
void func_800C4E50(void *item);
typedef struct SceneParticleOffset {
    s16 x;
    s16 y;
    s16 z;
    s16 pad06;
} SceneParticleOffset;
typedef struct SceneParticleSlots {
    u8 pad00[8];
    SceneParticleOffset offset[4];
    s32 phase[4];
    s16 verticalOffset[4];
    u8 pad40[8];
    s16 timer[4];
    s16 cycle[4];
    s16 motionRamp;
    s16 elapsedFrames;
} SceneParticleSlots;
typedef struct SceneEmitterSlots {
    SceneParticleOffset position[4];
    s16 field20[4];
    s16 field28[4];
    u8 field30[4];
    u8 field34[4];
    u8 field38[4];
    u8 field3C[4];
    u8 field40[4];
    u8 activeSlots;
} SceneEmitterSlots;
typedef struct SceneMovingVector {
    s16 x;
    s16 y;
    s16 z;
    s16 pad06;
    s16 combinedX;
    s16 combinedY;
    s16 combinedZ;
    s16 pad0E;
    s16 dx;
    s16 dy;
    s16 dz;
    s16 pad16;
} SceneMovingVector;
typedef struct SceneMovingSlots {
    SceneMovingVector vector[16];
    s32 life[16];
    s16 active[16];
    s16 timer[16];
    s16 initialSize[16];
    s16 angle[16];
    s16 range[16];
    s16 radius[16];
    s16 extraRadius[16];
} SceneMovingSlots;
typedef struct SceneMovingState {
    u8 pad00;
    u8 status;
    s16 elapsedFrames;
} SceneMovingState;
typedef struct SceneParticleCluster {
    SceneParticleOffset position[30];
    s16 fieldF0[30];
    s16 angle[30];
    s16 brightness[30];
    u8 texture[30];
    u8 mode[30];
    u8 counter[30];
    u8 field1FE[30];
    u8 active[30];
    u8 timer[30];
    u8 count;
} SceneParticleCluster;
void func_8018F958(int unused0, int unused1, SceneParticleCluster *slots) {
    unsigned int i;
    for (i = 0; i < 30; ++i) {
        {
            int randomValue = Engine_Random();
            slots->position[i].y = 0;
            slots->position[i].x = randomValue % 800 - 200;
        }
        slots->position[i].z = Engine_Random() % 600 - 300;
        slots->angle[i] = Engine_Random() % 4096;
        slots->counter[i] = 0;
        slots->field1FE[i] = 0;
        slots->active[i] = 0;
        slots->timer[i] = Engine_Random() % 50 + 2;
        if (i >= 4) {
            if ((Engine_Random() & 1) == 0) {
                slots->brightness[i] = 0xFF;
                slots->texture[i] = 0x4C;
                slots->mode[i] = 5;
                slots->fieldF0[i] = Engine_Random() % 256 + 0x200;
            } else {
                slots->brightness[i] = 0xFF;
                slots->texture[i] = 0x58;
                slots->mode[i] = 6;
                slots->fieldF0[i] = Engine_Random() % 512 + 0x300;
            }
        } else {
            slots->brightness[i] = 0x80;
            slots->texture[i] = 0x44;
            slots->mode[i] = 2;
            slots->fieldF0[i] = 0x7C;
        }
    }
    slots->count = 30;
    {
        void *volatile *sound = &D_800B0E64;
        if (*sound) {
            func_8006DF50(*sound, 0x608, 0, 0x80, 0x7F);
        }
    }
}
