#ifndef PE_PLAT_GPU_PRIMITIVE_H
#define PE_PLAT_GPU_PRIMITIVE_H
#include <stdint.h>

uint32_t pe_plat_gpu_draw_mode(int dither, int draw_to_display, uint32_t page);
int pe_plat_gpu_draw_offset(int x, int y);
void pe_plat_gpu_draw_offset_packet(uint32_t packet, uint32_t coords);
void pe_plat_gpu_poly_f3(uint32_t packet);
void pe_plat_gpu_poly_ft4(uint32_t packet);
void pe_plat_gpu_poly_g4(uint32_t packet);
void pe_plat_gpu_sprite(uint32_t packet);
void pe_plat_gpu_tile(uint32_t packet);
void pe_plat_gpu_line_f2(uint32_t packet);
void pe_plat_gpu_poly_g3(uint32_t packet);
void pe_plat_gpu_poly_gt4(uint32_t packet);
void pe_plat_gpu_tile1(uint32_t packet);
uint32_t pe_plat_gpu_draw_page(uint32_t packet, uint32_t dither,
                               uint32_t draw_to_display, uint32_t page);
void pe_plat_gpu_set_semitrans(uint32_t packet, int enabled);
void pe_plat_gpu_set_shade_texture(uint32_t packet, int enabled);
int pe_plat_gpu_append_packet(uint32_t head, uint32_t tail);
uint32_t pe_plat_gpu_display_env(uint32_t env, int x, int y, int width, int height);
/* GP0(E3h)/(E4h)/(E2h) words and ordering-table link (pe_plat_gpu_words.c,
 * clean-room from black-box vectors: pc_port/tools/pe_gpu_leaf_vectors.py). */
uint32_t pe_plat_gpu_draw_area_tl_word(short x, short y);
uint32_t pe_plat_gpu_draw_area_br_word(short x, short y);
uint32_t pe_plat_gpu_tex_window_word(uint32_t rect);
void pe_plat_gpu_add_prim(uint32_t ot, uint32_t prim);
/* SetDefDrawEnv (pe_plat_gpu_drawenv.c, clean-room from black-box vectors:
 * pc_port/tools/pe_gpu_leaf_vectors.py drawenv). */
uint32_t pe_plat_gpu_set_def_draw_env(uint32_t env, int x, int y, int w, int h);

#define func_80076170 pe_plat_gpu_draw_area_tl_word
#define func_80076208 pe_plat_gpu_draw_area_br_word
#define func_800762BC pe_plat_gpu_tex_window_word
#define func_80077AC4 pe_plat_gpu_add_prim
#define func_80074924 pe_plat_gpu_set_def_draw_env
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
uint32_t func_80076170(short x, short y);
uint32_t func_80076208(short x, short y);
uint32_t func_800762BC(uint32_t rect);
void func_80077AC4(uint32_t ot, uint32_t prim);
uint32_t func_80074924(uint32_t env, int x, int y, int w, int h);
/* libgpu helpers 0x80077548 / 0x80077A00 / 0x80076C10 (pe_plat_gpu_gen.c,
 * clean-room from black-box vectors: pe_cd_leaf_vectors.py cdgen5). */
int pe_plat_gpu_2b04(int a0);
void pe_plat_gpu_2fbc(void);
void pe_plat_gpu_21cc(int a0, int a1, int a2);
#define func_80077548 pe_plat_gpu_2b04
#define func_80077A00 pe_plat_gpu_2fbc
#define func_80076C10 pe_plat_gpu_21cc
int func_80077548(int a0);
void func_80077A00(void);
void func_80076C10(int a0, int a1, int a2);
/* libgpu ResetGraph core 0x80077144 (pe_plat_gpu_gen9.c, clean-room from
 * black-box vectors: pe_cd_leaf_vectors.py cdgen9). */
int pe_plat_gpu_2700(int mode);
#define func_80077144 pe_plat_gpu_2700
int func_80077144(int mode);
/* libgpu SetGraphDebug 0x80074BB8 (pe_plat_gpu_gen12.c, clean-room from
 * black-box vectors: pe_cd_leaf_vectors.py cdgen12). */
unsigned char pe_plat_gpu_set_graph_debug(unsigned char level);
/* libgpu 0x800719C4 (pe_plat_gpu_gen15.c, clean-room from black-box vectors:
 * pe_cd_leaf_vectors.py cdgen15): a flag-tested record field address. */
uint32_t pe_plat_gpu_flag_field(uint32_t rec);
/* libgpu 0x800718D0 (pe_plat_gpu_tim.c, clean-room from black-box vectors:
 * pe_gpu_tim_vectors.py): upload a TIM image's pixel and palette blocks. */
uint32_t pe_plat_gpu_load_tim(uint32_t tim);
#define func_800718D0 pe_plat_gpu_load_tim
#define func_80074BB8 pe_plat_gpu_set_graph_debug
#define func_800719C4 pe_plat_gpu_flag_field
unsigned char func_80074BB8(unsigned char a0);
uint32_t func_800719C4(uint32_t rec);
uint32_t func_800718D0(uint32_t tim);
#endif
