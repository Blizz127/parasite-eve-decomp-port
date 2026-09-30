/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/main/render/Render_SetViewport.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `Render_SetViewport` renamed to `func_80065E48`, and vendor symbol
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
int func_80077DC4(int angle);
int func_80077CF4(int angle);
extern int *D_800BCFA8;
int func_80065E48(s16 *position) {
    struct {
        s16 point[4];
        s16 projected[2];
        unsigned reserved[10];
    } local;
    GeomScrollState *camera = &D_800BCF88;
    register GeomState *geometry;
    register CameraViewport *view;
    register unsigned int cx asm("$11");
    register int correction asm("$2");
    register unsigned fallback_height asm("$2");
    register int fallback_half asm("$3");
    register int height_center asm("$3");
    register GeomState *offset_geometry;
    int flags = camera->flags;
    register int screen_x asm("$6");
    register int screen_y asm("$7");
    register int center_x asm("$6");
    register int center_y asm("$7");
    register int sum_x;
    register int sum_y;
    int view_x;
    register int view_y;
    register int half_width;
    int half_height;
    register int signed_height asm("$2");
    register int signed_width asm("$3");
    register int width_offset asm("$2");
    register unsigned raw_width;
    register unsigned raw_height asm("$3");
    register int target_x asm("$9");
    register int target_y;
    register int bound asm("$3");
    register int y_bound;
    register int selected_x;
    if (flags & 7)
        return -23;
    cx = 160;
    if (!(flags & 64))
        return -24;
    {
        int value = position[1];
        local.point[0] = value;
    }
    {
        int value = position[3];
        local.point[1] = value;
    }
    {
        int value = position[5];
        local.point[2] = value;
    }
    {
        register unsigned int cy asm("$15") = 112;
        register unsigned int x asm("$12");
        register unsigned int y asm("$13");
        asm("" : "=r"(cx), "=r"(cy) : "0"(cx), "1"(cy), "m"(local.point[2]));
        x = cx << 16;
        y = cy << 16;
        asm volatile("ctc2 %0,$24" : : "r"( x )) ;
        asm volatile("ctc2 %0,$25" : : "r"( y )) ;
    }
    {
        register u32 **address = &camera->position.matrixWords;
        register u32 *matrix asm("$11");
        register u32 x asm("$12");
        register u32 y asm("$13");
        register u32 z asm("$14");
        asm("" : "=r"(address) : "0"(address));
        matrix = *address;
        x = matrix[0];
        y = matrix[1];
        asm volatile("ctc2 %0,$0" : : "r"( x )) ;
        asm volatile("ctc2 %0,$1" : : "r"( y )) ;
        x = matrix[2];
        y = matrix[3];
        z = matrix[4];
        asm volatile("ctc2 %0,$2" : : "r"( x )) ;
        asm volatile("ctc2 %0,$3" : : "r"( y )) ;
        asm volatile("ctc2 %0,$4" : : "r"( z )) ;
        x = matrix[5];
        y = matrix[6];
        asm volatile("ctc2 %0,$5" : : "r"( x )) ;
        z = matrix[7];
        asm volatile("ctc2 %0,$6" : : "r"( y )) ;
        asm volatile("ctc2 %0,$7" : : "r"( z )) ;
    }
    {
        register int distance = *D_800BCFA8;
        asm volatile("ctc2 %0,$26" : : "r"( distance )) ;
    }
    asm volatile("lwc2 $0,0(%0)"  : : "r"( local.point ) : "memory") ;
    asm volatile("lwc2 $1,4(%0)"  : : "r"( local.point ) : "memory") ;
    asm volatile("nop") ;
    asm volatile("nop") ;
    asm volatile(".word 0x4A180001") ;
    asm volatile("swc2 $14,0(%0)" : : "r"( local.projected ) : "memory") ;
    screen_x = local.projected[0];
    screen_y = local.projected[1];
    if (camera->flags & 128) {
        D_800BCFB4 = screen_x;
        D_800BCFB6 = screen_y;
    }
    geometry = D_800B1624;
    sum_x = D_800BCFB4 + screen_x;
    center_x = sum_x / 2;
    D_800BCFB4 = center_x;
    sum_y = D_800BCFB6 + screen_y;
    sum_y += (unsigned)sum_y >> 31;
    view_x = 320 - center_x;
    offset_geometry = D_800B1624;
    center_y = sum_y >> 1;
    D_800BCFB6 = center_y;
    asm volatile("" : : : "memory");
    view = (CameraViewport *)((u8 *)geometry + offset_geometry->entry_offset_1C);
    view += D_800BCFFD;
    asm("" : : "r"(view) : "$3");
    height_center = 224;
    raw_width = view->width;
    view_y = height_center - center_y;
    raw_width <<= 16;
    signed_width = (int)raw_width >> 16;
    signed_width += raw_width >> 31;
    half_width = signed_width >> 1;
    width_offset = half_width - 160;
    raw_height = view->height;
    target_x = width_offset + center_x;
    raw_height <<= 16;
    signed_height = (int)raw_height >> 16;
    signed_height += raw_height >> 31;
    half_height = signed_height >> 1;
    target_y = half_height + (center_y - 112);
    bound = view->minX;
    if (target_x < bound)
        goto adjust_x;
    bound = view->maxX;
    if (bound < target_x)
        goto adjust_x;
    goto check_y;
adjust_x:
    correction = bound - 160;
    view_x = half_width - correction;
check_y:
    y_bound = view->minY;
    if (target_y < y_bound)
        goto adjust_y;
    y_bound = view->maxY;
    if (y_bound < target_y)
        goto adjust_y;
    goto store_offsets;
adjust_y:
    fallback_height = view->height;
    fallback_half = (s16)fallback_height / 2;
    correction = y_bound - 112;
    view_y = fallback_half - correction;
store_offsets:
    D_800BCF94 = view_x;
    D_800BCF96 = view_y;
    selected_x = target_x;
    if (D_800BCF88.flags & 64) {
        register int signed_x asm("$3");
        register int signed_y;
        register int loaded_bound asm("$2");
        register int chosen_bound asm("$5");
        register int upper_y;
        geometry = D_800B1624;
        view = (CameraViewport *)((u8 *)geometry + geometry->entry_offset_1C);
        view += D_800BCFFD;
        signed_x = (s16)target_x;
        loaded_bound = view->minX;
        chosen_bound = loaded_bound;
        asm("" : "=r"(chosen_bound) : "0"(chosen_bound));
        if (signed_x < loaded_bound) {
            signed_y = (unsigned)target_y << 16;
            D_800BCF8C.x = chosen_bound;
        } else {
            loaded_bound = view->maxX;
            chosen_bound = loaded_bound;
            asm("" : "=r"(chosen_bound) : "0"(chosen_bound));
            if (loaded_bound < signed_x) {
                signed_y = (unsigned)target_y << 16;
                D_800BCF8C.x = chosen_bound;
            } else {
                signed_y = (unsigned)target_y << 16;
                D_800BCF8C.x = selected_x;
            }
        }
        signed_y >>= 16;
        loaded_bound = view->minY;
        chosen_bound = loaded_bound;
        asm("" : "=r"(chosen_bound) : "0"(chosen_bound));
        if (signed_y < loaded_bound) {
            D_800BCF8E = chosen_bound;
            return 0;
        }
        loaded_bound = view->maxY;
        upper_y = loaded_bound;
        asm("" : "=r"(upper_y) : "0"(upper_y));
        if (loaded_bound < signed_y) {
            D_800BCF8E = upper_y;
            return 0;
        }
        D_800BCF8E = target_y;
    }
    return 0;
}
