/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/overlays/scene_e11/func_8018FD30.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `func_8018FD30` renamed to `func_8018FD28`, and vendor symbol
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
int func_80077DC4 (int angle);
int func_80077CF4 (int angle);
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
typedef struct RoomPairedTransformStack {
    RoomFxSeed8 seed;
    RoomSpriteMatrix scaleMatrix;
    RoomSpriteMatrix matrix;
    RoomFxVec4 scale;
    RoomFxVec4 sourceScale;
} RoomPairedTransformStack;
char *func_800C2B50(void);
void func_80071A44(RoomFxVec4 *vec, int value, int shift);
void func_80078CC4(RoomSpriteMatrix *matrix, RoomFxVec4 *scale);
void func_800794C4(RoomFxSeed8 *seed, RoomSpriteMatrix *matrix);
void func_800C3238(int page);
void func_800C3134(void *table, int step, void *out);
void func_800C4FC4(void *state, RoomSpriteMatrix *matrix, int mode);
extern RoomFxSeed8 D_8018EFF4;
extern unsigned char D_80191AD8 [];
void func_8018FD28  (void *unused, char *params,
                                          char *state) {
    RoomPairedTransformStack stack;
    register RoomSpriteMatrix *scaleMatrix asm("$16");
    register unsigned char *table asm("$17");
    register char *workState asm("$18");
    register RoomSpriteMatrix *matrix asm("$19");
    register char *root asm("$20");
    register char *renderParams asm("$21");
    register char *owner asm("$16");
    renderParams = params;
    workState = state;
    root = func_800C2B50();
    stack.seed = D_8018EFF4;
    stack.scaleMatrix.m[2][2] = 0x1000;
    stack.scaleMatrix.m[1][1] = 0x1000;
    stack.scaleMatrix.m[0][0] = 0x1000;
    stack.scaleMatrix.t[2] = 0;
    stack.scaleMatrix.t[1] = 0;
    stack.scaleMatrix.t[0] = 0;
    stack.scaleMatrix.m[2][1] = 0;
    stack.scaleMatrix.m[2][0] = 0;
    stack.scaleMatrix.m[1][2] = 0;
    stack.scaleMatrix.m[1][0] = 0;
    stack.scaleMatrix.m[0][2] = 0;
    stack.scaleMatrix.m[0][1] = 0;
    func_80071A44(&stack.sourceScale, 0, 0x10);
    stack.sourceScale.x = *(s16 *)(workState + 0x138) * 2;
    stack.sourceScale.y = *(s16 *)(workState + 0x138) * 2;
    stack.sourceScale.z = *(s16 *)(workState + 0x138) * 2;
    stack.scale = stack.sourceScale;
    scaleMatrix = &stack.scaleMatrix;
    func_80078CC4(scaleMatrix, &stack.scale);
    {
        register RoomFxSeed8 *seedArg asm("$4") = &stack.seed;
        asm("" : : "r"(seedArg));
        matrix = &stack.matrix;
        func_800794C4(seedArg, matrix);
    }
    asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( scaleMatrix ) : "$12", "$13", "$14") ;
    asm volatile("lhu $12,0(%0)\n\t" "lhu $13,6(%0)\n\t" "lhu $14,12(%0)\n\t" "mtc2 $12,$9\n\t" "mtc2 $13,$10\n\t" "mtc2 $14,$11\n\t" "nop\n\t" "nop\n\t" ".word 0x4A49E012" : : "r"( &stack.matrix.m[0][0] ) : "$12", "$13", "$14", "memory") ;
    do { register int x asm("$12"); register int y asm("$13"); register int z asm("$14"); asm volatile("mfc2 %0,$9\n\t" "mfc2 %1,$10\n\t" "mfc2 %2,$11" : "=r"(x), "=r"(y), "=r"(z), "=r"( matrix ) : "3"( matrix )); ((short *)( matrix ))[0] = x; ((short *)( matrix ))[3] = y; ((short *)( matrix ))[6] = z; } while (0) ;
    asm volatile("lhu $12,0(%0)\n\t" "lhu $13,6(%0)\n\t" "lhu $14,12(%0)\n\t" "mtc2 $12,$9\n\t" "mtc2 $13,$10\n\t" "mtc2 $14,$11\n\t" "nop\n\t" "nop\n\t" ".word 0x4A49E012" : : "r"( &stack.matrix.m[0][1] ) : "$12", "$13", "$14", "memory") ;
    do { volatile short *out = (volatile short *)( &stack.matrix.m[0][1] ); do { register int x asm("$12"); register int y asm("$13"); register int z asm("$14"); asm volatile("mfc2 %0,$9\n\t" "mfc2 %1,$10\n\t" "mfc2 %2,$11" : "=r"(x), "=r"(y), "=r"(z), "=r"( out ) : "3"( out )); ((short *)( out ))[0] = x; ((short *)( out ))[3] = y; ((short *)( out ))[6] = z; } while (0) ; } while (0) ;
    asm volatile("lhu $12,0(%0)\n\t" "lhu $13,6(%0)\n\t" "lhu $14,12(%0)\n\t" "mtc2 $12,$9\n\t" "mtc2 $13,$10\n\t" "mtc2 $14,$11\n\t" "nop\n\t" "nop\n\t" ".word 0x4A49E012" : : "r"( &stack.matrix.m[0][2] ) : "$12", "$13", "$14", "memory") ;
    do { volatile short *out = (volatile short *)( &stack.matrix.m[0][2] ); do { register int x asm("$12"); register int y asm("$13"); register int z asm("$14"); asm volatile("mfc2 %0,$9\n\t" "mfc2 %1,$10\n\t" "mfc2 %2,$11" : "=r"(x), "=r"(y), "=r"(z), "=r"( out ) : "3"( out )); ((short *)( out ))[0] = x; ((short *)( out ))[3] = y; ((short *)( out ))[6] = z; } while (0) ; } while (0) ;
    asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( scaleMatrix ) : "$12", "$13", "$14") ;
    {
        unsigned short *translation = (unsigned short *)stack.matrix.t;
        do { register unsigned int packed asm("$12"); register unsigned int high asm("$13"); asm volatile("" : "=r"( translation ) : "0"( translation )); high = ((volatile unsigned short *) translation )[2]; packed = ((unsigned short *) translation )[0]; asm volatile("" : "=r"(high) : "0"(high), "r"(packed)); high <<= 16; packed |= high; asm volatile("mtc2 %0,$0\n\t" "lwc2 $1,8(%1)" : : "r"(packed), "r"( translation ) : "memory"); } while (0) ;
        asm volatile("nop") ;
        asm volatile("nop") ;
        asm volatile(".word 0x4A480012") ;
        asm volatile("swc2 $9,0(%0)" : : "r"( translation ) : "memory") ;
        asm volatile("swc2 $10,4(%0)" : : "r"( translation ) : "memory") ;
        asm volatile("swc2 $11,8(%0)" : : "r"( translation ) : "memory") ;
    }
    func_800C3238(2);
    table = D_80191AD8 ;
    owner = *(char **)(*(char **)root + 0x238);
    func_800C3134(table, *(s16 *)(renderParams + 2), workState + 0x10C);
    func_800C3134(table, *(s16 *)(renderParams + 2), workState + 0x124);
    *(s16 *)(workState + 0x11C) = *(u16 *)(workState + 0x13A);
    stack.matrix.t[0] = *(int *)(owner + 0x14);
    stack.matrix.t[1] = *(int *)(owner + 0x18) - 0x64;
    stack.matrix.t[2] = *(int *)(owner + 0x1C);
    func_800C4FC4(workState + 0x108, matrix, 0);
}
