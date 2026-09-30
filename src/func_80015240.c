/* vendor-derived: khasinski/parasite-eve-decomp@3067919e7b82f0334b873117eef07e7157f4ed69 src/main/task/Task_SetGteMatrix.c — attribution khasinski (Chris Hasinski) */
/* inline GTE/asm: this function uses the vendor's inline PSY-Q GTE (COP2) asm macros — count separately from plain C. */
/* Imported under the owner's explicit authorization (2026-09-28, via Devbox Coordinator, vendor lane).
 * Upstream tree kept verbatim in vendor/khasinski-parasite-eve-decomp/ (see its PROVENANCE.md).
 * This file is that vendor TU run through the era cpp with the vendor CPP flags
 * (-undef -P, -I include, -I tools/m2c), `Task_SetGteMatrix` renamed to `func_80015240`, and vendor symbol
 * names mapped (post-cpp, token-wise) to this repo's address names from the vendor sym tables. */
/* vendor markers: CC1_FLAGS: -G8; MASPSX_FLAGS: -G8 */
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
typedef union RenderLightingMatrix {
    GteMatrix matrix;
    u32 words[8];
} RenderLightingMatrix;
extern RenderLightingMatrix D_800BEA40, D_800BEA60;
extern s16 D_800BEA42, D_800BEA44, D_800BEA46, D_800BEA48, D_800BEA4A;
extern s16 D_800BEA4C, D_800BEA4E, D_800BEA50;
extern s16 D_800BEA62, D_800BEA64, D_800BEA66, D_800BEA68, D_800BEA6A;
extern s16 D_800BEA6C, D_800BEA6E, D_800BEA70;
extern u8 D_800BD025, D_800BD026, D_800BD027;
int func_8006698C(void *object);
int func_80077DC4(int angle);
int func_80077CF4(int angle);
extern u8 *D2F0_setup[] __asm__("D_8009D2F0");
extern u8 *D2F0_matrix[] __asm__("D_8009D2F0");
extern u8 *D2F0_axis1[] __asm__("D_8009D2F0");
extern u8 *D2F0_axis2[] __asm__("D_8009D2F0");
extern u8 *D2F0_translation[] __asm__("D_8009D2F0");
extern u8 *D2F0_color0[] __asm__("D_8009D2F0");
extern u8 *D2F0_color1[] __asm__("D_8009D2F0");
extern u8 *D2F0_color2[] __asm__("D_8009D2F0");
extern u8 *D2F0_render0[] __asm__("D_8009D2F0");
extern u8 *D2F0_render1[] __asm__("D_8009D2F0");
extern u8 *D2F0_render2[] __asm__("D_8009D2F0");
extern u8 *D2F0_render3[] __asm__("D_8009D2F0");
extern u8 *D2F0_render4[] __asm__("D_8009D2F0");
extern u8 *D2F0_render5[] __asm__("D_8009D2F0");
extern u8 *D2F0_flags[] __asm__("D_8009D2F0");
extern u8 *D2F0_redraw[] __asm__("D_8009D2F0");
typedef struct GlobalIntSlot {
    int value;
    u8 pad[8];
} GlobalIntSlot;
extern GlobalIntSlot CDDC_draw0 __asm__("D_8009CDDC");
extern GlobalIntSlot CDDC_toggle0_load __asm__("D_8009CDDC");
extern GlobalIntSlot CDDC_toggle0_store __asm__("D_8009CDDC");
extern GlobalIntSlot CDDC_draw1 __asm__("D_8009CDDC");
extern GlobalIntSlot CDDC_toggle1_load __asm__("D_8009CDDC");
extern GlobalIntSlot CDDC_toggle1_store __asm__("D_8009CDDC");
extern int *D_8009CE00;
extern u8 *D_8009D300;
extern u32 D_800B89F8[];
void func_800794C4(GteRotation *rotation, GteMatrix *matrix);
void func_80039B74(u8 *object, u8 *animation, int frame, int mode);
void func_8003A088(u8 *object);
void func_8003A6A8(u8 *object, u32 *view_matrix);
void func_8003B97C(u8 *object, u32 *prim_state);
void func_8003BCE0(u8 *object, int force, int buffer_index);
int func_80015240(int **args) {
    u8 *setup_actor;
    u8 *matrix_actor;
    u8 *color_actor;
    u8 *render_actor;
    u8 *flag_actor;
    GteMatrix scale_matrix;
    register GteMatrix *first_matrix asm("$3");
    GteMatrix *second_matrix;
    GteMatrix *third_matrix;
    GteMatrix *translation_matrix;
    int first_draw_slot;
    int second_draw_slot;
    int result;
    int *script_ptr;
    u8 *task_state;
    u32 flags;
    setup_actor = D2F0_setup[0];
    (*(s32 *)((u8 *)( setup_actor ) + (  0x1FC )))  = (*(s16 *)((u8 *)( setup_actor ) + (  0x2A ))) ;
    (*(s32 *)((u8 *)( setup_actor ) + (  0x200 )))  = (*(s16 *)((u8 *)( setup_actor ) + (  0x2E ))) ;
    (*(s32 *)((u8 *)( setup_actor ) + (  0x204 )))  = (*(s16 *)((u8 *)( setup_actor ) + (  0x32 ))) ;
    (*(u16 *)((u8 *)( setup_actor ) + (  0x1E0 )))  = (*(u16 *)((u8 *)( setup_actor ) + (  0x38 ))) ;
    (*(u16 *)((u8 *)( setup_actor ) + (  0x1E2 )))  = (*(u16 *)((u8 *)( setup_actor ) + (  0x3A ))) ;
    (*(u16 *)((u8 *)( setup_actor ) + (  0x1E4 )))  = (*(u16 *)((u8 *)( setup_actor ) + (  0x3C ))) ;
    func_800794C4((GteRotation *)(setup_actor + 0x1E0),
              (GteMatrix *)(setup_actor + 0x1E8));
    matrix_actor = D2F0_matrix[0];
    scale_matrix.m[0][0] = (*(u16 *)((u8 *)( matrix_actor ) + (  0x26 ))) ;
    scale_matrix.m[1][1] = (*(u16 *)((u8 *)( matrix_actor ) + (  0x26 ))) ;
    scale_matrix.m[2][2] = (*(u16 *)((u8 *)( matrix_actor ) + (  0x26 ))) ;
    first_matrix = (GteMatrix *)(matrix_actor + 0x1E8);
    scale_matrix.t[2] = 0;
    scale_matrix.t[1] = 0;
    scale_matrix.t[0] = 0;
    scale_matrix.m[2][1] = 0;
    scale_matrix.m[2][0] = 0;
    scale_matrix.m[1][2] = 0;
    scale_matrix.m[1][0] = 0;
    scale_matrix.m[0][2] = 0;
    scale_matrix.m[0][1] = 0;
    asm volatile("lw $12,0(%0)\n\t" "lw $13,4(%0)\n\t" "ctc2 $12,$0\n\t" "ctc2 $13,$1\n\t" "lw $12,8(%0)\n\t" "lw $13,12(%0)\n\t" "lw $14,16(%0)\n\t" "ctc2 $12,$2\n\t" "ctc2 $13,$3\n\t" "ctc2 $14,$4" : : "r"( first_matrix ) : "$12", "$13", "$14") ;
    asm volatile("lhu $12,0(%0)\n\t" "lhu $13,6(%0)\n\t" "lhu $14,12(%0)\n\t" "mtc2 $12,$9\n\t" "mtc2 $13,$10\n\t" "mtc2 $14,$11\n\t" "nop\n\t" "nop\n\t" ".word 0x4A49E012" : : "r"( &scale_matrix.m[0][0] ) : "$12", "$13", "$14", "memory") ;
    do { register int x asm("$12"); register int y asm("$13"); register int z asm("$14"); asm volatile("mfc2 %0,$9\n\t" "mfc2 %1,$10\n\t" "mfc2 %2,$11" : "=r"(x), "=r"(y), "=r"(z), "=r"( first_matrix ) : "3"( first_matrix )); ((short *)( first_matrix ))[0] = x; ((short *)( first_matrix ))[3] = y; ((short *)( first_matrix ))[6] = z; } while (0) ;
    asm volatile("lhu $12,0(%0)\n\t" "lhu $13,6(%0)\n\t" "lhu $14,12(%0)\n\t" "mtc2 $12,$9\n\t" "mtc2 $13,$10\n\t" "mtc2 $14,$11\n\t" "nop\n\t" "nop\n\t" ".word 0x4A49E012" : : "r"( &scale_matrix.m[0][1] ) : "$12", "$13", "$14", "memory") ;
    second_matrix = (GteMatrix *)(D2F0_axis1[0] + 0x1E8);
    do { volatile short *out = (volatile short *)( &second_matrix->m[0][1] ); do { register int x asm("$12"); register int y asm("$13"); register int z asm("$14"); asm volatile("mfc2 %0,$9\n\t" "mfc2 %1,$10\n\t" "mfc2 %2,$11" : "=r"(x), "=r"(y), "=r"(z), "=r"( out ) : "3"( out )); ((short *)( out ))[0] = x; ((short *)( out ))[3] = y; ((short *)( out ))[6] = z; } while (0) ; } while (0) ;
    asm volatile("lhu $12,0(%0)\n\t" "lhu $13,6(%0)\n\t" "lhu $14,12(%0)\n\t" "mtc2 $12,$9\n\t" "mtc2 $13,$10\n\t" "mtc2 $14,$11\n\t" "nop\n\t" "nop\n\t" ".word 0x4A49E012" : : "r"( &scale_matrix.m[0][2] ) : "$12", "$13", "$14", "memory") ;
    third_matrix = (GteMatrix *)(D2F0_axis2[0] + 0x1E8);
    do { volatile short *out = (volatile short *)( &third_matrix->m[0][2] ); do { register int x asm("$12"); register int y asm("$13"); register int z asm("$14"); asm volatile("mfc2 %0,$9\n\t" "mfc2 %1,$10\n\t" "mfc2 %2,$11" : "=r"(x), "=r"(y), "=r"(z), "=r"( out ) : "3"( out )); ((short *)( out ))[0] = x; ((short *)( out ))[3] = y; ((short *)( out ))[6] = z; } while (0) ; } while (0) ;
    translation_matrix = (GteMatrix *)(D2F0_translation[0] + 0x1E8);
    asm volatile("lw $12,20(%0)\n\t" "lw $13,24(%0)\n\t" "ctc2 $12,$5\n\t" "lw $14,28(%0)\n\t" "ctc2 $13,$6\n\t" "ctc2 $14,$7" : : "r"( translation_matrix ) : "$12", "$13", "$14") ;
    do { unsigned short *source = (unsigned short *)( scale_matrix.t ); do { register unsigned int packed asm("$12"); register unsigned int high asm("$13"); asm volatile("" : "=r"( source ) : "0"( source )); high = ((volatile unsigned short *) source )[2]; packed = ((unsigned short *) source )[0]; asm volatile("" : "=r"(high) : "0"(high), "r"(packed)); high <<= 16; packed |= high; asm volatile("mtc2 %0,$0\n\t" "lwc2 $1,8(%1)" : : "r"(packed), "r"( source ) : "memory"); } while (0) ; } while (0) ;
    asm volatile("nop") ;
    asm volatile("nop") ;
    asm volatile(".word 0x4A480012") ;
    asm volatile("swc2 $9,0(%0)" : : "r"( translation_matrix->t ) : "memory") ;
    asm volatile("swc2 $10,4(%0)" : : "r"( translation_matrix->t ) : "memory") ;
    asm volatile("swc2 $11,8(%0)" : : "r"( translation_matrix->t ) : "memory") ;
    color_actor = D2F0_color0[0];
    (*(u8 *)((u8 *)( color_actor ) + (  0x23C )))  = *args[0];
    (*(u8 *)((u8 *)( D2F0_color1[0] ) + (  0x23D )))  = *args[1];
    (*(u8 *)((u8 *)( D2F0_color2[0] ) + (  0x23E )))  = *args[2];
    func_8006698C(D2F0_render0[0] + 0x1B4);
    render_actor = D2F0_render1[0];
    func_80039B74(render_actor + 0x1B4,
                               (*(u8 **)((u8 *)( render_actor ) + (  0x1B0 ))) , 0, 1);
    func_8003A088(D2F0_render2[0] + 0x1B4);
    func_8003A6A8(D2F0_render3[0] + 0x1B4, D_800B89F8);
    func_8003B97C(D2F0_render4[0] + 0x1B4, D_800BEA40.words);
    func_8003BCE0(D2F0_render5[0] + 0x1B4, 1,
                           (s16)CDDC_draw0.value);
    flags = (*(u32 *)((u8 *)( flag_actor = D2F0_flags[0] ) + (  0x98 ))) ;
    if ((flags & 0x10000000) != 0) {
        first_draw_slot = CDDC_toggle0_load.value;
        first_draw_slot ^= 1;
        CDDC_toggle0_store.value = first_draw_slot;
        func_8003B97C(flag_actor + 0x1B4, D_800BEA40.words);
        func_8003BCE0(D2F0_redraw[0] + 0x1B4, 1,
                               (s16)CDDC_draw1.value);
        second_draw_slot = CDDC_toggle1_load.value;
        second_draw_slot ^= 1;
        CDDC_toggle1_store.value = second_draw_slot;
        goto success;
    }
    if ((flags & 0x08000000) == 0) {
        result = 0;
        script_ptr = D_8009CE00;
        (*(u32 *)((u8 *)( flag_actor ) + (  0x98 )))  = flags | 0x08000000;
        task_state = D_8009D300;
        D_8009CE00 = script_ptr - 5;
        (*(s32 *)((u8 *)( task_state ) + (  0x10 )))  = 1;
        return result;
    }
    (*(u32 *)((u8 *)( flag_actor ) + (  0x98 )))  = flags & 0xF7FFFFFF;
success:
    return 1;
}
