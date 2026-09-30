/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/room_m269/RoomEffect_RenderMotionSprite.c — attribution khasinski (Chris Hasinski) */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `RoomEffect_RenderMotionSprite` renamed to `func_8018F800`, and vendor symbol
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
    u16 x;
    u16 y;
    u16 z;
    u16 pad;
    s16 blocked;
    u16 frame;
} MotionSpriteState;
typedef char pe1_static_assert_motion_sprite_blocked_offset [( ((u32)&((( MotionSpriteState  *)0)->  blocked ))  == 0x08 ) ? 1 : -1] ;
typedef char pe1_static_assert_motion_sprite_frame_offset [( ((u32)&((( MotionSpriteState  *)0)->  frame ))  == 0x0A ) ? 1 : -1] ;
typedef struct {
    s16 x;
    s16 y;
    s16 z;
    s16 pad;
} SpritePosition;
extern RoomFxSeed8 D_8018F004;
extern s32 D_800E27EC;
extern s32 D_800F3428;
extern u16 D_800F336C;
extern s16 D_800F336A;
extern u16 D_800E1204[];
u16 func_80077AA4(s32 x, s32 y);
void func_800CEE20(SpritePosition *position, void *rotation, s32 scale_x,
                   s32 scale_y, s32 height, s32 clut, s32 mode, s32 shade,
                   void *color);
int func_8018F800(int mode, MotionSpriteState *state) {
    SpritePosition position;
    RoomFxSeed8 seed = D_8018F004;
    int shade;
    int palette;
    int kind;
    u16 clut;
    switch (mode) {
    case 1: {
        int next;
        int signed_next;
        if (state->blocked) return 0;
        next = state->frame + 1;
        state->frame = next;
        asm volatile("" : "=r"(next) : "0"(next) : "memory");
        signed_next = (s16)next;
        asm volatile("" : "=r"(signed_next) : "0"(signed_next));
        if (signed_next < 16) return 0;
        asm volatile("" ::: "memory");
        return 1;
    }
    case 2:
        break;
    default:
        return 0;
    }
    if (state->blocked) return 0;
    shade = (D_800E27EC & 1) ? 128 : 100;
    position.x = state->x;
    position.y = state->y;
    position.z = state->z;
    if ((s16)state->frame < 8) {
        register int first_kind asm("$2");
        first_kind = D_800F336C;
        palette = D_800E1204[first_kind];
        if (first_kind == 4 && D_800F3428) palette += 4;
        clut = func_80077AA4(16, palette);
        func_800CEE20(&position, &seed, 4096, 4096,
                        D_800F336A * (s16)state->frame + 32,
                        clut, 1, (unsigned)shade >> 1, 0);
    }
    position.y -= 40;
    kind = D_800F336C;
    palette = D_800E1204[kind];
    if (kind == 4 && D_800F3428) palette += 4;
    clut = func_80077AA4(48, palette);
    func_800CEE20(&position, 0, 4096, 4096,
                    D_800F336A * ((s16)state->frame / 2) + 96,
                    clut, 1, (unsigned)shade >> 1, 0);
    return 0;
}
