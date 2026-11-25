/* GTE control-register setters (guest 0x80078E04 / 0x80078E34 / 0x80078E64 /
 * 0x80078E94 / 0x80078FC4 / 0x80078FE4 / 0x80079004 / 0x80079024) and
 * ScaleMatrix (guest 0x80078CC4), written clean-room from black-box vectors
 * of the original code (pc_port/tools/pe_gte_leaf_vectors.py), with no SDK
 * source.  Evidence: docs/evidence/native-gte-setters-20261030/. */
#include "pe_plat/gte.h"
#include "pe_guest_ram.h"
#include "pe_sdk.h"

/* GTE control registers touched here. */
#define GTE_CTRL_ROT_0      0u  /* rotation matrix: 5 words, ctrl 0..4 */
#define GTE_CTRL_TRANS_X    5u  /* translation vector: ctrl 5..7 */
#define GTE_CTRL_LIGHT_0    8u  /* light matrix: ctrl 8..12 */
#define GTE_CTRL_BACK_R     13u /* background colour: ctrl 13..15 */
#define GTE_CTRL_COLOR_0    16u /* colour matrix: ctrl 16..20 */
#define GTE_CTRL_FAR_R      21u /* far colour: ctrl 21..23 */
#define GTE_CTRL_OFFSET_X   24u /* screen offset X, Y: ctrl 24, 25 */
#define GTE_CTRL_OFFSET_Y   25u
#define GTE_CTRL_SCREEN_H   26u /* projection plane distance */

/* Guest matrix: five words of packed halfwords (3x3 s16, row-major), then
 * three words of translation. */
#define MATRIX_WORDS        5u
#define MATRIX_TRANS_OFF    0x14u
#define VECTOR_WORDS        3u

#define COLOR_SHIFT         4   /* colour args are stored <<4 */
#define OFFSET_SHIFT        16  /* screen offsets are stored <<16 */
#define SCALE_SHIFT         12  /* ScaleMatrix factor is 4.12 fixed point */

static void load_ctrl_words(uint32_t first, uint32_t addr, uint32_t count)
{
    uint32_t i;
    for (i = 0; i < count; i++)
        PE_GTE_WriteCtrl(first + i, PE_LoadU32(addr + 4u * i));
}

void pe_plat_gte_set_rot_matrix(uint32_t matrix)
{
    load_ctrl_words(GTE_CTRL_ROT_0, matrix, MATRIX_WORDS);
}

void pe_plat_gte_set_light_matrix(uint32_t matrix)
{
    load_ctrl_words(GTE_CTRL_LIGHT_0, matrix, MATRIX_WORDS);
}

void pe_plat_gte_set_color_matrix(uint32_t matrix)
{
    load_ctrl_words(GTE_CTRL_COLOR_0, matrix, MATRIX_WORDS);
}

void pe_plat_gte_set_trans_matrix(uint32_t matrix)
{
    load_ctrl_words(GTE_CTRL_TRANS_X, matrix + MATRIX_TRANS_OFF, VECTOR_WORDS);
}

void pe_plat_gte_set_back_color(uint32_t r, uint32_t g, uint32_t b)
{
    PE_GTE_WriteCtrl(GTE_CTRL_BACK_R, r << COLOR_SHIFT);
    PE_GTE_WriteCtrl(GTE_CTRL_BACK_R + 1u, g << COLOR_SHIFT);
    PE_GTE_WriteCtrl(GTE_CTRL_BACK_R + 2u, b << COLOR_SHIFT);
}

void pe_plat_gte_set_far_color(uint32_t r, uint32_t g, uint32_t b)
{
    PE_GTE_WriteCtrl(GTE_CTRL_FAR_R, r << COLOR_SHIFT);
    PE_GTE_WriteCtrl(GTE_CTRL_FAR_R + 1u, g << COLOR_SHIFT);
    PE_GTE_WriteCtrl(GTE_CTRL_FAR_R + 2u, b << COLOR_SHIFT);
}

void pe_plat_gte_set_geom_offset(int x, int y)
{
    PE_GTE_WriteCtrl(GTE_CTRL_OFFSET_X, (uint32_t)x << OFFSET_SHIFT);
    PE_GTE_WriteCtrl(GTE_CTRL_OFFSET_Y, (uint32_t)y << OFFSET_SHIFT);
}

void pe_plat_gte_set_geom_screen(int h)
{
    PE_GTE_WriteCtrl(GTE_CTRL_SCREEN_H, (uint32_t)h);
}

/* Signed 16-bit value of a packed word's low (0) or high (1) half. */
static int32_t half_s16(uint32_t word, unsigned hi)
{
    return (int16_t)(uint16_t)(word >> (16u * hi));
}

/* One matrix element times one scale word: the product is truncated to 32
 * bits (a plain 32-bit multiply), then shifted right arithmetically. */
static uint32_t scale_elem(int32_t elem, uint32_t factor)
{
    return (uint32_t)((int32_t)((uint32_t)elem * factor) >> SCALE_SHIFT);
}

/* Scales column j of the 3x3 matrix at `matrix` by word j of the 3-word
 * vector at `scale`.  Elements are packed two per word; the last element
 * (row 2, column 2) is stored as the whole unmasked result word. */
uint32_t pe_plat_gte_scale_matrix(uint32_t matrix, uint32_t scale)
{
    uint32_t f0 = PE_LoadU32(scale);
    uint32_t f1 = PE_LoadU32(scale + 4u);
    uint32_t f2 = PE_LoadU32(scale + 8u);
    uint32_t w0 = PE_LoadU32(matrix);
    uint32_t w1 = PE_LoadU32(matrix + 4u);
    uint32_t w2 = PE_LoadU32(matrix + 8u);
    uint32_t w3 = PE_LoadU32(matrix + 12u);
    uint32_t w4 = PE_LoadU32(matrix + 16u);

    /* words 0..3 hold elements 0..7, columns cycling 0,1,2 */
    uint32_t r0 = scale_elem(half_s16(w0, 0), f0);
    uint32_t r1 = scale_elem(half_s16(w0, 1), f1);
    uint32_t r2 = scale_elem(half_s16(w1, 0), f2);
    uint32_t r3 = scale_elem(half_s16(w1, 1), f0);
    uint32_t r4 = scale_elem(half_s16(w2, 0), f1);
    uint32_t r5 = scale_elem(half_s16(w2, 1), f2);
    uint32_t r6 = scale_elem(half_s16(w3, 0), f0);
    uint32_t r7 = scale_elem(half_s16(w3, 1), f1);
    uint32_t r8 = scale_elem(half_s16(w4, 0), f2);

    PE_StoreU32(matrix, (r0 & 0xFFFFu) | (r1 << 16));
    PE_StoreU32(matrix + 4u, (r2 & 0xFFFFu) | (r3 << 16));
    PE_StoreU32(matrix + 8u, (r4 & 0xFFFFu) | (r5 << 16));
    PE_StoreU32(matrix + 12u, (r6 & 0xFFFFu) | (r7 << 16));
    PE_StoreU32(matrix + 16u, r8);
    return matrix;
}
