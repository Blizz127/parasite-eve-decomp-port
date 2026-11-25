/* GTE matrix-command leaves (guest 0x800785D4 MulMatrix0 / 0x800786E4
 * MulRotMatrix / 0x800787D4 CompMatrix / 0x80078BE4 ApplyMatrix / 0x80078C34
 * ApplyMatrixSV), written clean-room from black-box vectors of the original
 * code (pc_port/tools/pe_gte_cmd_vectors.py), with no SDK source.  Evidence:
 * docs/evidence/native-gte-cmd-20261031/. */
#include "pe_plat/gte.h"
#include "pe_guest_ram.h"
#include "pe_sdk.h"
#include "game_port.h"

#define GTE_CTRL_ROT_0      0u  /* rotation matrix: 5 words, ctrl 0..4 */
#define GTE_DATA_VXY0       0u  /* input vector 0: x,y packed; z in VZ0 */
#define GTE_DATA_VZ0        1u
#define GTE_DATA_IR1        9u  /* result vector IR1..IR3 (saturated s16) */
#define GTE_DATA_MAC1       25u /* MAC1..MAC3: unsaturated 32-bit results */

/* cop2 MVMVA: rotation matrix * V0, no translation, shift 12, IR clamped to s16. */
#define GTE_CMD_MVMVA_R_V0  0x4A486012u

#define MATRIX_WORDS        5u  /* 3x3 s16 row-major packed in five words */
#define MATRIX_TRANS_OFF    0x14u

static int32_t half_s16(uint32_t word, unsigned hi)
{
    return (int16_t)(uint16_t)(word >> (16u * hi));
}

/* Element `idx` (0..8, row-major) of the packed 3x3 matrix whose words are
 * w[0..4]. */
static int32_t elem(const uint32_t *w, unsigned idx)
{
    return half_s16(w[idx >> 1], idx & 1u);
}

static void load_rot(uint32_t m)
{
    uint32_t i;
    for (i = 0; i < MATRIX_WORDS; i++)
        PE_GTE_WriteCtrl(GTE_CTRL_ROT_0 + i, PE_LoadU32(m + 4u * i));
}

static void set_v0(int32_t x, int32_t y, int32_t z)
{
    PE_GTE_WriteData(GTE_DATA_VXY0, ((uint32_t)y << 16) | ((uint32_t)x & 0xFFFFu));
    PE_GTE_WriteData(GTE_DATA_VZ0, (uint32_t)z);
}

/* Runs the matrix command on V0 = (x,y,z); the result is IR1..IR3 (and
 * MAC1..MAC3 when `mac` is non-NULL). */
static void mvmva(int32_t x, int32_t y, int32_t z, int32_t ir[3], uint32_t *mac)
{
    uint32_t i;
    set_v0(x, y, z);
    PE_GTE_Command(GTE_CMD_MVMVA_R_V0);
    for (i = 0; i < 3u; i++) {
        ir[i] = (int32_t)PE_GTE_ReadData(GTE_DATA_IR1 + i);
        if (mac)
            mac[i] = PE_GTE_ReadData(GTE_DATA_MAC1 + i);
    }
}

/* r = (rotation matrix in the GTE) * the matrix held in words w[0..4], one
 * column per command. */
static void mul_columns(const uint32_t *w, int32_t r[9])
{
    uint32_t c;
    for (c = 0; c < 3u; c++) {
        int32_t ir[3];
        mvmva(elem(w, c), elem(w, 3u + c), elem(w, 6u + c), ir, 0);
        r[c] = ir[0];
        r[3u + c] = ir[1];
        r[6u + c] = ir[2];
    }
}

/* Stores output word k (0..3: two packed elements; 4: element 8 as a whole
 * sign-extended word). */
static void store_word(uint32_t out, const int32_t r[9], unsigned k)
{
    uint32_t v = k == 4u ? (uint32_t)r[8]
                         : ((uint32_t)r[2u * k] & 0xFFFFu) | ((uint32_t)r[2u * k + 1u] << 16);
    PE_StoreU32(out + 4u * k, v);
}

static void read_matrix(uint32_t m, uint32_t *w)
{
    uint32_t i;
    for (i = 0; i < MATRIX_WORDS; i++)
        w[i] = PE_LoadU32(m + 4u * i);
}

uint32_t pe_plat_gte_mul_matrix0(uint32_t m0, uint32_t m1, uint32_t out)
{
    uint32_t w[MATRIX_WORDS];
    int32_t r[9];
    load_rot(m0);
    read_matrix(m1, w);
    mul_columns(w, r);
    store_word(out, r, 0);
    store_word(out, r, 3);
    store_word(out, r, 1);
    store_word(out, r, 2);
    store_word(out, r, 4);
    return out;
}

void pe_plat_gte_mul_rot_matrix(uint32_t m)
{
    uint32_t w[MATRIX_WORDS];
    int32_t r[9];
    read_matrix(m, w);
    mul_columns(w, r);
    store_word(m, r, 0);
    store_word(m, r, 3);
    store_word(m, r, 1);
    store_word(m, r, 2);
    store_word(m, r, 4);
}

uint32_t pe_plat_gte_comp_matrix(uint32_t m0, uint32_t m1, uint32_t out)
{
    uint32_t w[MATRIX_WORDS];
    int32_t ir[3];
    uint32_t mac[3];
    int32_t b[3];
    uint32_t t = m1 + MATRIX_TRANS_OFF;
    uint32_t i;
    int32_t r[9];
    load_rot(m0);
    read_matrix(m1, w);
    mul_columns(w, r);
    /* Words 1 and 2 are stored after the translation of m1 is read, so an
     * output overlapping m1's translation sees the original order. */
    store_word(out, r, 0);
    store_word(out, r, 3);
    store_word(out, r, 4);
    mvmva((int16_t)PE_LoadU16(t), (int16_t)PE_LoadU32(t + 4u),
          (int16_t)PE_LoadU32(t + 8u), ir, mac);
    store_word(out, r, 1);
    store_word(out, r, 2);
    /* Each translation word is a trapping signed add (all three addends are
     * read first); an overflow raises the overflow exception before any
     * translation word is stored. */
    for (i = 0; i < 3u; i++)
        b[i] = (int32_t)PE_LoadU32(m0 + MATRIX_TRANS_OFF + 4u * i);
    for (i = 0; i < 3u; i++) {
        int32_t sum = (int32_t)(mac[i] + (uint32_t)b[i]);
        if ((((int32_t)mac[i] ^ sum) & (b[i] ^ sum)) < 0) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return out;
        }
        mac[i] = (uint32_t)sum;
    }
    for (i = 0; i < 3u; i++)
        PE_StoreU32(out + MATRIX_TRANS_OFF + 4u * i, mac[i]);
    return out;
}

uint32_t pe_plat_gte_apply_matrix(uint32_t m, uint32_t v, uint32_t out)
{
    int32_t ir[3];
    uint32_t mac[3];
    uint32_t xy, z;
    load_rot(m);
    xy = PE_LoadU32(v);
    z = PE_LoadU32(v + 4u);
    mvmva(half_s16(xy, 0), half_s16(xy, 1), (int16_t)z, ir, mac);
    PE_StoreU32(out, mac[0]);
    PE_StoreU32(out + 4u, mac[1]);
    PE_StoreU32(out + 8u, mac[2]);
    return out;
}

uint32_t pe_plat_gte_apply_matrix_sv(uint32_t m, uint32_t v, uint32_t out)
{
    int32_t ir[3];
    uint32_t xy, z;
    load_rot(m);
    xy = PE_LoadU32(v);
    z = PE_LoadU32(v + 4u);
    mvmva(half_s16(xy, 0), half_s16(xy, 1), (int16_t)z, ir, 0);
    PE_StoreU16(out, (uint16_t)ir[0]);
    PE_StoreU16(out + 2u, (uint16_t)ir[1]);
    PE_StoreU16(out + 4u, (uint16_t)ir[2]);
    return out;
}
