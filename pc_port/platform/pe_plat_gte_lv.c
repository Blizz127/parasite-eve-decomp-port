/* GTE long-vector matrix leaf (guest 0x80078934 ApplyMatrixLV), written
 * clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_gte_cmd_vectors.py batch5), with no SDK source.
 * Evidence: docs/evidence/native-gte-applymatrixlv-20261031/. */
#include "pe_plat/gte.h"
#include "pe_guest_ram.h"
#include "pe_sdk.h"

#define GTE_CTRL_ROT_0      0u  /* rotation matrix: 5 words, ctrl 0..4 */
#define GTE_DATA_IR1        9u  /* IR1..IR3: command input vector (v = IR) */
#define GTE_DATA_MAC1       25u /* MAC1..MAC3: unsaturated 32-bit results */

/* cop2 MVMVA: rotation matrix * IR, no translation, lm = 0; sf = 0 keeps the
 * full product, sf = 1 shifts it right by 12. */
#define GTE_CMD_MVMVA_R_IR_SF0  0x0041E012u
#define GTE_CMD_MVMVA_R_IR_SF1  0x0049E012u

#define MATRIX_WORDS        5u  /* 3x3 s16 row-major packed in five words */
#define LV_SPLIT_BITS       15u /* magnitude split: |v| = hi * 2^15 + lo */
#define LV_LO_MASK          0x7FFFu
#define LV_HI_SCALE_SHIFT   3u  /* hi * M is scaled by 2^15 / 2^12 */

static void load_rot(uint32_t m)
{
    uint32_t i;
    for (i = 0; i < MATRIX_WORDS; i++)
        PE_GTE_WriteCtrl(GTE_CTRL_ROT_0 + i, PE_LoadU32(m + 4u * i));
}

/* Runs one command with IR1..IR3 = x[0..2] and reads MAC1..MAC3. */
static void run(uint32_t cmd, const int32_t x[3], uint32_t mac[3])
{
    uint32_t i;
    for (i = 0; i < 3u; i++)
        PE_GTE_WriteData(GTE_DATA_IR1 + i, (uint32_t)x[i]);
    PE_GTE_Command(cmd);
    for (i = 0; i < 3u; i++)
        mac[i] = PE_GTE_ReadData(GTE_DATA_MAC1 + i);
}

uint32_t pe_plat_gte_apply_matrix_lv(uint32_t m, uint32_t v, uint32_t out)
{
    int32_t hi[3], lo[3];
    uint32_t mh[3], ml[3];
    uint32_t i;
    load_rot(m);
    for (i = 0; i < 3u; i++) {
        int32_t x = (int32_t)PE_LoadU32(v + 4u * i);
        uint32_t a = x < 0 ? 0u - (uint32_t)x : (uint32_t)x;
        int32_t h = (int32_t)(a >> LV_SPLIT_BITS);
        int32_t l = (int32_t)(a & LV_LO_MASK);
        hi[i] = x < 0 ? -h : h;
        lo[i] = x < 0 ? -l : l;
    }
    run(GTE_CMD_MVMVA_R_IR_SF0, hi, mh);
    run(GTE_CMD_MVMVA_R_IR_SF1, lo, ml);
    for (i = 0; i < 3u; i++)
        PE_StoreU32(out + 4u * i, (mh[i] << LV_HI_SCALE_SHIFT) + ml[i]);
    return out;
}
