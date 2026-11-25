#ifndef PE_PLAT_GTE_H
#define PE_PLAT_GTE_H
#include <stdint.h>

uint32_t pe_plat_gte_translate_matrix(uint32_t matrix, uint32_t vector);
/* GTE register setters and ScaleMatrix (pe_plat_gte_regs.c, clean-room from
 * black-box vectors: pc_port/tools/pe_gte_leaf_vectors.py). */
void pe_plat_gte_set_rot_matrix(uint32_t matrix);
void pe_plat_gte_set_light_matrix(uint32_t matrix);
void pe_plat_gte_set_color_matrix(uint32_t matrix);
void pe_plat_gte_set_trans_matrix(uint32_t matrix);
void pe_plat_gte_set_back_color(uint32_t r, uint32_t g, uint32_t b);
void pe_plat_gte_set_far_color(uint32_t r, uint32_t g, uint32_t b);
void pe_plat_gte_set_geom_offset(int x, int y);
void pe_plat_gte_set_geom_screen(int h);
uint32_t pe_plat_gte_scale_matrix(uint32_t matrix, uint32_t scale);
/* PushMatrix / PopMatrix (pe_plat_gte_stack.c, clean-room from black-box
 * vectors: pc_port/tools/pe_gte_leaf_vectors.py stack). */
void pe_plat_gte_push_matrix(void);
void pe_plat_gte_pop_matrix(void);
/* rsin/rcos and the RotMatrix family (pe_plat_gte_trig.c, clean-room from
 * black-box vectors: pc_port/tools/pe_gte_trig_vectors.py). */
int pe_plat_gte_rsin(int a);
int pe_plat_gte_sin_index(int a);
int pe_plat_gte_rcos(int a);
void pe_plat_gte_rot_matrix(uint32_t angles, uint32_t out);
uint32_t pe_plat_gte_rot_matrix_zyx(uint32_t angles, uint32_t out);
void pe_plat_gte_rot_matrix_z(int32_t angle, uint32_t out);
uint32_t pe_plat_gte_rot_matrix_yxz(uint32_t angles, uint32_t out);
/* Host-array forms (angles x, y, z in; nine row-major halfwords out); the
 * tables are still read from guest memory. */
void pe_plat_gte_rot_matrix_values(const int16_t angles[3], int16_t out[9]);
void pe_plat_gte_rot_matrix_yxz_values(const int16_t angles[3], int16_t out[9]);
/* GTE-command leaves (pe_plat_gte_cmd.c, clean-room from black-box vectors:
 * pc_port/tools/pe_gte_cmd_vectors.py). */
uint32_t pe_plat_gte_mul_matrix0(uint32_t m0, uint32_t m1, uint32_t out);
void pe_plat_gte_mul_rot_matrix(uint32_t m);
uint32_t pe_plat_gte_comp_matrix(uint32_t m0, uint32_t m1, uint32_t out);
uint32_t pe_plat_gte_apply_matrix(uint32_t m, uint32_t v, uint32_t out);
uint32_t pe_plat_gte_apply_matrix_sv(uint32_t m, uint32_t v, uint32_t out);
/* ApplyMatrixLV (pe_plat_gte_lv.c, clean-room: pe_gte_cmd_vectors.py batch5). */
uint32_t pe_plat_gte_apply_matrix_lv(uint32_t m, uint32_t v, uint32_t out);
/* Batch 2 (pe_plat_gte_cmd2.c, same vector tool, `batch2`). */
void pe_plat_gte_load_average_short12(uint32_t v0, uint32_t v1, int32_t p0, int32_t p1, uint32_t out);
void pe_plat_gte_load_average_col(uint32_t v0, uint32_t v1, int32_t p0, int32_t p1, uint32_t out);
void pe_plat_gte_outer_product0(uint32_t v0, uint32_t v1, uint32_t out);
int32_t pe_plat_gte_rot_trans_pers(uint32_t v0, uint32_t sxy, uint32_t p, uint32_t flag);
int32_t pe_plat_gte_rot_trans_pers3(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t sxy0,
                                    uint32_t sxy1, uint32_t sxy2, uint32_t p, uint32_t flag);
uint32_t pe_plat_gte_rot_trans(uint32_t v0, uint32_t out, uint32_t flag);
int32_t pe_plat_gte_rot_average_nclip3(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t sxy0,
                                       uint32_t sxy1, uint32_t sxy2, uint32_t p, uint32_t otz,
                                       uint32_t flag);
int32_t pe_plat_gte_rot_average_nclip4(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3,
                                       uint32_t sxy0, uint32_t sxy1, uint32_t sxy2, uint32_t sxy3,
                                       uint32_t p, uint32_t otz, uint32_t flag);
/* Batch 3 (pe_plat_gte_div.c, same vector tool, `batch3`). */
void pe_plat_gte_set_fog_near_far(uint32_t near_z, uint32_t far_z, int32_t h);
uint32_t pe_plat_gte_square_root0(uint32_t value);
int32_t pe_plat_gte_ratan2(int32_t y, int32_t x);
/* InitGeom (pe_plat_gte_init.c, clean-room from psx-spx + the host GTE model). */
void pe_plat_gte_init_geom(void);
/* Batch 4 (pe_plat_gte_vn.c, same vector tool, `batch4`). */
uint32_t pe_plat_gte_vector_normal(uint32_t in, uint32_t out);

#define func_80078C94 pe_plat_gte_translate_matrix
#define func_80078E04 pe_plat_gte_set_rot_matrix
#define func_80078E34 pe_plat_gte_set_light_matrix
#define func_80078E64 pe_plat_gte_set_color_matrix
#define func_80078E94 pe_plat_gte_set_trans_matrix
#define func_80078FC4 pe_plat_gte_set_back_color
#define func_80078FE4 pe_plat_gte_set_far_color
#define func_80079004 pe_plat_gte_set_geom_offset
#define func_80079024 pe_plat_gte_set_geom_screen
#define func_80078CC4 pe_plat_gte_scale_matrix
#define func_80078A94 pe_plat_gte_push_matrix
#define func_80078B38 pe_plat_gte_pop_matrix
#define func_80077CF4 pe_plat_gte_rsin
#define func_80077D30 pe_plat_gte_sin_index
#define func_80077DC4 pe_plat_gte_rcos
#define func_800794C4 pe_plat_gte_rot_matrix
#define func_800799E4 pe_plat_gte_rot_matrix_zyx
#define func_80079E14 pe_plat_gte_rot_matrix_z
#define func_800786E4 pe_plat_gte_mul_rot_matrix
#define func_800787D4 pe_plat_gte_comp_matrix
#define func_80078C34 pe_plat_gte_apply_matrix_sv
#define func_80078934 pe_plat_gte_apply_matrix_lv
#define func_800783E4 pe_plat_gte_load_average_short12
#define func_80078554 pe_plat_gte_load_average_col
#define func_800791D0 pe_plat_gte_outer_product0
#define func_80079244 pe_plat_gte_rot_trans_pers
#define func_80079274 pe_plat_gte_rot_trans_pers3
#define func_800792D4 pe_plat_gte_rot_trans
#define func_80079384 pe_plat_gte_rot_average_nclip3
#define func_80079414 pe_plat_gte_rot_average_nclip4
#define func_80077E64 pe_plat_gte_set_fog_near_far
#define func_80078004 pe_plat_gte_square_root0
#define func_80079FB4 pe_plat_gte_ratan2
#define func_80078134 pe_plat_gte_vector_normal
#define func_80077F7C pe_plat_gte_init_geom
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
uint32_t func_80078C94(uint32_t matrix, uint32_t vector);
void func_80078E04(uint32_t matrix);
void func_80078E34(uint32_t matrix);
void func_80078E64(uint32_t matrix);
void func_80078E94(uint32_t matrix);
void func_80078FC4(uint32_t r, uint32_t g, uint32_t b);
void func_80078FE4(uint32_t r, uint32_t g, uint32_t b);
void func_80079004(int x, int y);
void func_80079024(int h);
uint32_t func_80078CC4(uint32_t matrix, uint32_t scale);
void func_80078A94(void);
void func_80078B38(void);
int func_80077CF4(int a);
int func_80077D30(int a);
int func_80077DC4(int a);
void func_800794C4(uint32_t angles, uint32_t out);
uint32_t func_800799E4(uint32_t angles, uint32_t out);
void func_80079E14(int32_t angle, uint32_t out);
void func_800786E4(uint32_t m);
uint32_t func_800787D4(uint32_t m0, uint32_t m1, uint32_t out);
uint32_t func_80078C34(uint32_t m, uint32_t v, uint32_t out);
void func_800783E4(uint32_t v0, uint32_t v1, int32_t p0, int32_t p1, uint32_t out);
void func_80078554(uint32_t v0, uint32_t v1, int32_t p0, int32_t p1, uint32_t out);
void func_800791D0(uint32_t v0, uint32_t v1, uint32_t out);
int32_t func_80079244(uint32_t v0, uint32_t sxy, uint32_t p, uint32_t flag);
int32_t func_80079274(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t sxy0, uint32_t sxy1, uint32_t sxy2, uint32_t p, uint32_t flag);
uint32_t func_800792D4(uint32_t v0, uint32_t out, uint32_t flag);
int32_t func_80079384(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t sxy0, uint32_t sxy1, uint32_t sxy2, uint32_t p, uint32_t otz, uint32_t flag);
int32_t func_80079414(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3, uint32_t sxy0, uint32_t sxy1, uint32_t sxy2, uint32_t sxy3, uint32_t p, uint32_t otz, uint32_t flag);
void func_80077E64(uint32_t near_z, uint32_t far_z, int32_t h);
uint32_t func_80078004(uint32_t value);
int32_t func_80079FB4(int32_t y, int32_t x);
uint32_t func_80078134(uint32_t in, uint32_t out);
void func_80077F7C(void);
uint32_t func_80078934(uint32_t m, uint32_t v, uint32_t out);
#endif
