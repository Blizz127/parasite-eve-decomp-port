/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/main/render/Render_UpdateScrollPosition.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `Render_UpdateScrollPosition` renamed to `func_800665A0`, and vendor symbol
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
typedef struct RenderGpuTag {
    u32 address : 24;
    u32 length : 8;
} RenderGpuTag;
typedef struct RenderTintTile {
    RenderGpuTag tag;
    u8 r, g, b, command;
    s16 x, y, width, height;
} RenderTintTile;
typedef struct RenderTintMode {
    RenderGpuTag tag;
    u32 command;
} RenderTintMode;
typedef struct RenderTintState {
    RenderTintTile tiles[2];
    RenderTintMode modes[2];
    s16 target_r, target_g, target_b;
    u8 fade_mode, blend_mode;
    s16 start_r, start_g, start_b;
    u16 duration, frame;
} RenderTintState;
typedef char pe1_static_assert_render_gpu_tag_size [( sizeof(RenderGpuTag) == 4 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tint_tile_size [( sizeof(RenderTintTile) == 16 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tint_tile_rgb_offset [( ((u32)&((( RenderTintTile  *)0)->  r ))  == 4 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tint_mode_size [( sizeof(RenderTintMode) == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tint_command_offset [( ((u32)&((( RenderTintMode  *)0)->  command ))  == 4 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tint_state_size [( sizeof(RenderTintState) == 0x44 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tint_modes_offset [( ((u32)&((( RenderTintState  *)0)->  modes ))  == 0x20 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tint_target_offset [( ((u32)&((( RenderTintState  *)0)->  target_r ))  == 0x30 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tint_flags_offset [( ((u32)&((( RenderTintState  *)0)->  fade_mode ))  == 0x36 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tint_start_offset [( ((u32)&((( RenderTintState  *)0)->  start_r ))  == 0x38 ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tint_duration_offset [( ((u32)&((( RenderTintState  *)0)->  duration ))  == 0x3E ) ? 1 : -1] ;
typedef char pe1_static_assert_render_tint_frame_offset [( ((u32)&((( RenderTintState  *)0)->  frame ))  == 0x40 ) ? 1 : -1] ;
extern s16 D_800BCFE8, D_800BCFEA, D_800BCFEC;
extern s16 D_800BCFF0, D_800BCFF2, D_800BCFF4;
extern u8 D_800BCFEE;
extern u16 D_800BCFF6, D_800BCFF8;
int func_80068E24(void);
typedef struct GeomCtrlEntry {             
    union {                                
        u32 packed;
        struct { u8 flags; u8 _b[3]; } b;
    } head;
    u16 field4;                            
    s16 field6;                            
    s16 field8;                            
    u16 fieldA;                            
    s32 slot_offset;                       
} GeomCtrlEntry;
typedef struct GeomAnimationSlot {
    u8 entry;
    s8 duration;
} GeomAnimationSlot;
typedef struct GeomAnimationControl {
    union { u32 packed; struct { u8 flags; u8 padding[3]; } b; } head;
    unsigned int group : 8;
    signed int position : 24;
    s16 step;
    u16 elapsed;
    s32 slotOffset;
} GeomAnimationControl;
typedef char pe1_static_assert_geom_animation_slot_size [( sizeof(GeomAnimationSlot) == 2 ) ? 1 : -1] ;
typedef char pe1_static_assert_geom_animation_control_view_size [( sizeof(GeomAnimationControl) == sizeof(GeomCtrlEntry) ) ? 1 : -1] ;
typedef char pe1_static_assert_geom_animation_step_offset [( ((u32)&((( GeomAnimationControl  *)0)->  step ))  == 8 ) ? 1 : -1] ;
typedef char pe1_static_assert_geom_animation_elapsed_offset [( ((u32)&((( GeomAnimationControl  *)0)->  elapsed ))  == 10 ) ? 1 : -1] ;
typedef char pe1_static_assert_geom_animation_slots_offset [( ((u32)&((( GeomAnimationControl  *)0)->  slotOffset ))  == 12 ) ? 1 : -1] ;
struct RenderTexturePagePacket;
typedef struct GeomEntry {                 
    u8  flags;                             
    u8  pad01[3];                          
    u16 anim_mod_x;                        
    u16 anim_mod_y;                        
    u16 base_x;                            
    u16 base_y;                            
    u16 scr_x;                             
    u16 scr_y;                             
    s16 ot10;                              
    s16 ot12;                              
    s16 ot14;                              
    s16 ot16;                              
    u16 disp_x;                            
    u16 disp_y;                            
    s16 field1C;                           
    s16 field1E;                           
    s16 field20;                           
    s16 field22;                           
    u8  group;                             
    u8  pad25[1];                          
    u16 prim_count;                        
    union {                                
        s32 pos_ptr;
        struct { u16 size28; u16 size2A; } sz;
    } u28;
    union {                                
        s32 uv_ptr;
        struct { s16 min_x; s16 max_x; } bx;
    } u2C;
    union {                                
        void *prim;
        struct { s16 min_y; s16 max_y; } by;
    } u30;
    union {                                
        struct RenderTexturePagePacket *pagePackets;
        u8 storage[4];
    } u34;
} GeomEntry;
typedef char pe1_static_assert_geom_entry_page_offset [( ((u32)&((( GeomEntry  *)0)->  u34 ))  == 0x34 ) ? 1 : -1] ;
struct RenderTilePacket;
int func_80066F60(GeomEntry *entry, struct RenderTilePacket *buffer, void **end);
typedef union GeomScrollFraction {
    u16 word;
    u8 byte;
} GeomScrollFraction;
typedef struct GeomScrollEntry {
    u8 flags, padding[3];
    u16 modulusX, modulusY;
    s16 baseX, baseY, x, y;
    u8 reserved10[12];
    s16 speedX, speedY;
    GeomScrollFraction fractionX, fractionY;
    u8 reserved24[20];
} GeomScrollEntry;
typedef struct GeomScrollCoordinates {
    s16 x, y;
    u16 savedX, savedY;
    u16 screenOffsetX, screenOffsetY;
    u16 startX, startY;
    u16 targetX, targetY;
    u16 elapsed, duration;
    u32 *matrixWords;
    u8 reserved1C[0x10];
    RenderTintState tint;
    u8 reserved70[0x2C];
    s16 originX, originY;
} GeomScrollCoordinates;
typedef struct GeomScrollState {
    u32 flags;
    GeomScrollCoordinates position;
} GeomScrollState;
typedef char pe1_static_assert_geom_scroll_entry_size [( sizeof(GeomScrollEntry) == 0x38 ) ? 1 : -1] ;
typedef char pe1_static_assert_geom_scroll_x_offset [( ((u32)&((( GeomScrollEntry  *)0)->  x ))  == 0x0C ) ? 1 : -1] ;
typedef char pe1_static_assert_geom_scroll_speed_offset [( ((u32)&((( GeomScrollEntry  *)0)->  speedX ))  == 0x1C ) ? 1 : -1] ;
typedef char pe1_static_assert_geom_scroll_fraction_offset [( ((u32)&((( GeomScrollEntry  *)0)->  fractionX ))  == 0x20 ) ? 1 : -1] ;
typedef char pe1_static_assert_geom_scroll_position_offset [( ((u32)&((( GeomScrollState  *)0)->  position ))  == 4 ) ? 1 : -1] ;
typedef char pe1_static_assert_geom_scroll_origin_offset [( ((u32)&((( GeomScrollCoordinates  *)0)->  originX ))  == 0x9C ) ? 1 : -1] ;
typedef char pe1_static_assert_geom_scroll_tint_offset [( ((u32)&((( GeomScrollState  *)0)->  position.tint ))  == 0x30 ) ? 1 : -1] ;
typedef struct GeomState {                 
    u8  pad00[4];                          
    u16 entry_count;                       
    u16 entry_count06;                     
    u8  pad08[8];                          
    s32 ctrl_offset;                       
    s32 entry_offset;                      
    u8  pad18[4];                          
    s32 entry_offset_1C;                   
    u8  pad20[4];                          
    u16 depth_offset;                      
    u16 field26;                           
    u16 clip_offset_x;                     
    u16 clip_offset_y;                     
    u16 disp_src_x;                        
    u16 disp_src_y;                        
    s16 clip_min_x;                        
    s16 clip_max_x;                        
    s16 clip_min_y;                        
    s16 clip_max_y;                        
    s16 out_disp_x;                        
    s16 out_disp_y;                        
} GeomState;
typedef char pe1_static_assert_geom_depth_offset [( ((u32)&((( GeomState  *)0)->  depth_offset ))  == 0x24 ) ? 1 : -1] ;
typedef char pe1_static_assert_geom_clip_offset_x [( ((u32)&((( GeomState  *)0)->  clip_offset_x ))  == 0x28 ) ? 1 : -1] ;
typedef char pe1_static_assert_geom_clip_offset_y [( ((u32)&((( GeomState  *)0)->  clip_offset_y ))  == 0x2A ) ? 1 : -1] ;
extern GeomState * volatile g_GeomState asm("D_800B1624");
extern GeomState * volatile D_800B1624;
extern u8 D_800BCFFD;
int func_80065674(void);
int func_80067E1C(void);
int func_80066268(void);
typedef struct CameraViewport {
    u8 prefix[40];
    u16 width, height;
    s16 minX, maxX, minY, maxY;
} CameraViewport;
typedef char pe1_static_assert_camera_viewport_size [( sizeof(CameraViewport) == 52 ) ? 1 : -1] ;
typedef char pe1_static_assert_camera_viewport_width_offset [( ((u32)&((( CameraViewport  *)0)->  width ))  == 40 ) ? 1 : -1] ;
typedef char pe1_static_assert_camera_viewport_min_x_offset [( ((u32)&((( CameraViewport  *)0)->  minX ))  == 44 ) ? 1 : -1] ;
typedef char pe1_static_assert_camera_matrix_words_offset [( ((u32)&((( GeomScrollCoordinates  *)0)->  matrixWords ))  == 24 ) ? 1 : -1] ;
int func_800665A0(void *position, int duration, int mode);
extern s16 D_800BCFFE;
extern int g_RenderStateFlags asm("D_800BCF88");
extern u16 D_800BCF94, D_800BCF96, D_800BCF98, D_800BCF9A;
extern u16 D_800BCF9C, D_800BCF9E, D_800BCFA0, D_800BCFA2;
extern GeomScrollState D_800BCF88;
extern GeomScrollCoordinates D_800BCF8C;
extern u16 D_800BCF8E, D_800BCF90, D_800BCF92;
int func_800671C8(GeomEntry *entry, int x, int y, int depth);
int func_800679C4(int x, int y, int z);
int func_80077DC4(int angle);
int func_80077CF4(int angle);
int func_800665A0(void *positionArg, int duration, int mode)
{
    int *position = positionArg;
    short vector[4];
    short projected[2];
    GeomScrollState *camera = &D_800BCF88;
    CameraViewport *view;
    int x, y;
    if (!(camera->flags & 64))
        return -21;
    D_800BCF98 = D_800BCF8C.x;
    D_800BCF9A = D_800BCF8E;
    asm volatile("" : : : "memory");
    vector[0] = position[0]>>16;
    vector[1] = (position[1]>>16)-(u16)D_800BCFFE;
    vector[2] = position[2]>>16;
    {
        register volatile unsigned short cx asm("$10") = 160;
        register unsigned int cy asm("$11") = 112;
        register unsigned int sx asm("$12");
        register unsigned int sy asm("$13");
        asm("" : "=r"(cx), "=r"(cy) : "0"(cx), "1"(cy) : "memory");
        sx = cx<<16;
        sy = cy<<16;
        asm volatile("ctc2 %0,$24" : : "r"( sx )) ;
        asm volatile("ctc2 %0,$25" : : "r"( sy )) ;
    }
    {
        register unsigned int **address asm("$2") = &camera->position.matrixWords;
        register unsigned int *matrix asm("$10");
        register unsigned int a asm("$12");
        register unsigned int b asm("$13");
        register unsigned int c asm("$14");
        asm("" : "=r"(address) : "0"(address));
        matrix = *address;
        a = matrix[0];
        b = matrix[1];
        asm("" : : "r"(a), "r"(b));
        asm volatile("ctc2 %0,$0" : : "r"( a )) ;
        asm volatile("ctc2 %0,$1" : : "r"( b )) ;
        a = matrix[2];
        b = matrix[3];
        c = matrix[4];
        asm("" : : "r"(a), "r"(b), "r"(c));
        asm volatile("ctc2 %0,$2" : : "r"( a )) ;
        asm volatile("ctc2 %0,$3" : : "r"( b )) ;
        asm volatile("ctc2 %0,$4" : : "r"( c )) ;
        a = matrix[5];
        b = matrix[6];
        asm("" : : "r"(a), "r"(b));
        asm volatile("ctc2 %0,$5" : : "r"( a )) ;
        c = matrix[7];
        asm volatile("ctc2 %0,$6" : : "r"( b )) ;
        asm volatile("ctc2 %0,$7" : : "r"( c )) ;
    }
    asm volatile("lwc2 $0,0(%0)"  : : "r"( vector ) : "memory") ;
    asm volatile("lwc2 $1,4(%0)"  : : "r"( vector ) : "memory") ;
    asm volatile("nop") ;
    asm volatile("nop") ;
    asm volatile(".word 0x4A180001") ;
    asm volatile("swc2 $14,0(%0)" : : "r"( projected ) : "memory") ;
    {
        GeomState *state = D_800B1624;
        CameraViewport *views = (CameraViewport *)((u8 *)D_800B1624+state->entry_offset_1C);
        view = &views[D_800BCFFD];
    }
    {
        u16 width = view->width,height = view->height;
        int halfX = (short)width/2-160,halfY = (short)height/2-112;
        x = halfX+projected[0];
        y = halfY+projected[1];
    }
    {
        int min = view->minX;
        if (x < min) x = min;
        else {
            int max = view->maxX;
            if (x > max) x = max;
        }
    }
    {
        int min = view->minY;
        if (y < min) y = min;
        else {
            int max = view->maxY;
            if (y > max) y = max;
        }
    }
    D_800BCF9C = x;
    D_800BCF9E = y;
    if (duration == -1)
        D_800BCFA2 = 30;
    else
        D_800BCFA2 = duration;
    D_800BCFA0 = 1;
    if (mode == -1)
        g_RenderStateFlags = (g_RenderStateFlags & ~7) | 3;
    else {
        unsigned int flags = g_RenderStateFlags & ~15;
        unsigned int bits = mode | 3;
        g_RenderStateFlags = flags | bits;
    }
    return 0;
}
