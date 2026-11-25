/* Default draw-environment initialiser (guest 0x80074924), written clean-room
 * from black-box vectors of the original code
 * (pc_port/tools/pe_gpu_leaf_vectors.py drawenv), with no SDK source.
 * Evidence: docs/evidence/native-gpu-drawenv-20261031/. */
#include "pe_plat/gpu_primitive.h"
#include "pe_guest_ram.h"

/* Video-mode word: the vectors show zero selects the short (256-line) limit
 * below, every other value (1, 2, 0xFFFFFFFF, seeded) the tall one. */
#define GPU_VIDEO_MODE         0x800956ECu

/* Environment record field offsets (halfwords unless noted). */
#define ENV_CLIP_X             0x00u /* low 16 bits of x */
#define ENV_CLIP_Y             0x02u /* low 16 bits of y */
#define ENV_CLIP_W             0x04u /* low 16 bits of w */
#define ENV_CLIP_H             0x06u /* low 16 bits of h */
#define ENV_OFS_X              0x08u /* copy of x (low 16 bits) */
#define ENV_OFS_Y              0x0Au /* copy of y (low 16 bits) */
#define ENV_ZERO_C             0x0Cu /* always 0 */
#define ENV_ZERO_E             0x0Eu /* always 0 */
#define ENV_ZERO_10            0x10u /* always 0 */
#define ENV_ZERO_12            0x12u /* always 0 */
#define ENV_CONST_14           0x14u /* always 0x000A */
#define ENV_CONST_16           0x16u /* byte, always 1 */
#define ENV_FLAG_17            0x17u /* byte, 1 when h is within the limit */
#define ENV_ZERO_18            0x18u /* bytes, always 0 */
#define ENV_ZERO_19            0x19u
#define ENV_ZERO_1A            0x1Au
#define ENV_ZERO_1B            0x1Bu

#define ENV_CONST_14_VALUE     0x000Au

/* ENV_FLAG_17 is 1 when h, compared as a signed 32-bit value (so negative h
 * counts as within), is at most 256 for video mode 0 and 288 otherwise; the
 * vectors cover both sides of each boundary (256/257 and 288/289). */
#define GPU_H_LIMIT_MODE0      256
#define GPU_H_LIMIT_OTHER      288

uint32_t pe_plat_gpu_set_def_draw_env(uint32_t env, int x, int y, int w, int h)
{
    int limit = (PE_LoadU32(GPU_VIDEO_MODE) == 0u) ? GPU_H_LIMIT_MODE0
                                                   : GPU_H_LIMIT_OTHER;

    PE_StoreU16(env + ENV_CLIP_X, (uint16_t)x);
    PE_StoreU16(env + ENV_CLIP_Y, (uint16_t)y);
    PE_StoreU16(env + ENV_CLIP_W, (uint16_t)w);
    PE_StoreU16(env + ENV_ZERO_C, 0u);
    PE_StoreU16(env + ENV_ZERO_E, 0u);
    PE_StoreU16(env + ENV_ZERO_10, 0u);
    PE_StoreU16(env + ENV_ZERO_12, 0u);
    PE_StoreU8(env + ENV_ZERO_19, 0u);
    PE_StoreU8(env + ENV_ZERO_1A, 0u);
    PE_StoreU8(env + ENV_ZERO_1B, 0u);
    PE_StoreU8(env + ENV_CONST_16, 1u);
    PE_StoreU16(env + ENV_CLIP_H, (uint16_t)h);
    PE_StoreU8(env + ENV_FLAG_17, (uint8_t)(h <= limit));
    PE_StoreU16(env + ENV_OFS_X, (uint16_t)x);
    PE_StoreU16(env + ENV_OFS_Y, (uint16_t)y);
    PE_StoreU16(env + ENV_CONST_14, ENV_CONST_14_VALUE);
    PE_StoreU8(env + ENV_ZERO_18, 0u);
    return env;
}
