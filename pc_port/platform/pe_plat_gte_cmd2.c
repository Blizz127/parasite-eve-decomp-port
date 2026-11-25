/* GTE vertex / blend leaves, batch 2 (guest 0x800783E4 LoadAverageShort12 /
 * 0x80078554 LoadAverageCol / 0x800791D0 OuterProduct0 / 0x80079244
 * RotTransPers / 0x80079274 RotTransPers3 / 0x800792D4 RotTrans /
 * 0x80079384 RotAverageNclip3 / 0x80079414 RotAverageNclip4), written
 * clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_gte_cmd_vectors.py batch2), with no SDK source.
 * Evidence: docs/evidence/native-gte-cmd-20261031/. */
#include "pe_plat/gte.h"
#include "pe_guest_ram.h"
#include "pe_sdk.h"

#define GTE_CTRL_ROT_11     0u  /* RT11 | RT12<<16 */
#define GTE_CTRL_ROT_22     2u  /* RT22 | RT23<<16 */
#define GTE_CTRL_ROT_33     4u  /* RT33 */
#define GTE_CTRL_FLAG       31u

#define GTE_DATA_VXY0       0u  /* input vertices: (x,y) packed, then z */
#define GTE_DATA_VZ0        1u
#define GTE_DATA_IR0        8u
#define GTE_DATA_IR1        9u
#define GTE_DATA_OTZ        7u
#define GTE_DATA_SXY0       12u /* screen xy FIFO: SXY0, SXY1, SXY2 */
#define GTE_DATA_SXY2       14u
#define GTE_DATA_SZ3        19u
#define GTE_DATA_MAC0       24u
#define GTE_DATA_MAC1       25u

#define GTE_CMD_RTPS        0x4A180001u
#define GTE_CMD_RTPT        0x4A280030u
#define GTE_CMD_MVMVA_R_V0_TR 0x4A480012u
#define GTE_CMD_NCLIP       0x4A400006u
#define GTE_CMD_AVSZ3       0x4A58002Du
#define GTE_CMD_AVSZ4       0x4A68002Eu
#define GTE_CMD_OP          0x4A70000Cu
#define GTE_CMD_GPF_SF12    0x4A98003Du /* IR * IR0 >> 12 */
#define GTE_CMD_GPL_SF12    0x4AA8003Eu
#define GTE_CMD_GPF_RAW     0x4A90003Du /* IR * IR0, kept in MAC */
#define GTE_CMD_GPL_RAW     0x4AA0003Eu

static uint32_t sext16(uint32_t v)
{
    return (uint32_t)(int32_t)(int16_t)(uint16_t)v;
}

/* Loads the short vertex at `v` (two words: x|y<<16, then z) into the
 * vertex register pair `n` (0..2). */
static void load_vertex(unsigned n, uint32_t v)
{
    PE_GTE_WriteData(GTE_DATA_VXY0 + 2u * n, PE_LoadU32(v));
    PE_GTE_WriteData(GTE_DATA_VZ0 + 2u * n, PE_LoadU32(v + 4u));
}

static void set_ir(unsigned n, uint32_t v)
{
    PE_GTE_WriteData(GTE_DATA_IR1 + n, sext16(v));
}

static uint32_t sxy(unsigned n)
{
    return PE_GTE_ReadData(GTE_DATA_SXY0 + n);
}

/* IR1..IR3 = the three shorts packed in two words at `v`. */
static void load_ir_shorts(uint32_t v)
{
    uint32_t xy = PE_LoadU32(v);
    uint32_t z = PE_LoadU32(v + 4u);
    set_ir(0, xy);
    set_ir(1, xy >> 16);
    set_ir(2, z);
}

/* IR1..IR3 = three bytes at `v`, zero-extended. */
static void load_ir_bytes(uint32_t v)
{
    unsigned i;
    for (i = 0; i < 3u; i++)
        PE_GTE_WriteData(GTE_DATA_IR1 + i, PE_LoadU8(v + i));
}

void pe_plat_gte_load_average_short12(uint32_t v0, uint32_t v1, int32_t p0, int32_t p1, uint32_t out)
{
    uint32_t ir1, ir2, ir3;
    PE_GTE_WriteData(GTE_DATA_IR0, sext16((uint32_t)p0));
    load_ir_shorts(v0);
    PE_GTE_Command(GTE_CMD_GPF_SF12);
    PE_GTE_WriteData(GTE_DATA_IR0, sext16((uint32_t)p1));
    load_ir_shorts(v1);
    PE_GTE_Command(GTE_CMD_GPL_SF12);
    ir1 = PE_GTE_ReadData(GTE_DATA_IR1);
    ir2 = PE_GTE_ReadData(GTE_DATA_IR1 + 1u);
    ir3 = PE_GTE_ReadData(GTE_DATA_IR1 + 2u);
    PE_StoreU32(out, (ir1 & 0xFFFFu) | (ir2 << 16));
    PE_StoreU32(out + 4u, ir3);
}

void pe_plat_gte_load_average_col(uint32_t v0, uint32_t v1, int32_t p0, int32_t p1, uint32_t out)
{
    unsigned i;
    PE_GTE_WriteData(GTE_DATA_IR0, sext16((uint32_t)p0));
    load_ir_bytes(v0);
    PE_GTE_Command(GTE_CMD_GPF_RAW);
    PE_GTE_WriteData(GTE_DATA_IR0, sext16((uint32_t)p1));
    load_ir_bytes(v1);
    PE_GTE_Command(GTE_CMD_GPL_RAW);
    for (i = 0; i < 3u; i++)
        PE_StoreU8(out + i, (uint8_t)((int32_t)PE_GTE_ReadData(GTE_DATA_MAC1 + i) >> 12));
}

/* v0 x v1 of two 3-word vectors: the rotation matrix diagonal is borrowed to
 * hold v0 for the outer-product command and restored afterwards. */
void pe_plat_gte_outer_product0(uint32_t v0, uint32_t v1, uint32_t out)
{
    uint32_t c0 = PE_GTE_ReadCtrl(GTE_CTRL_ROT_11);
    uint32_t c2 = PE_GTE_ReadCtrl(GTE_CTRL_ROT_22);
    uint32_t c4 = PE_GTE_ReadCtrl(GTE_CTRL_ROT_33);
    unsigned i;
    PE_GTE_WriteCtrl(GTE_CTRL_ROT_11, (c0 & 0xFFFF0000u) | (PE_LoadU32(v0) & 0xFFFFu));
    PE_GTE_WriteCtrl(GTE_CTRL_ROT_22, (c2 & 0xFFFF0000u) | (PE_LoadU32(v0 + 4u) & 0xFFFFu));
    PE_GTE_WriteCtrl(GTE_CTRL_ROT_33, PE_LoadU32(v0 + 8u));
    for (i = 0; i < 3u; i++)
        set_ir(i, PE_LoadU32(v1 + 4u * i));
    PE_GTE_Command(GTE_CMD_OP);
    for (i = 0; i < 3u; i++)
        PE_StoreU32(out + 4u * i, PE_GTE_ReadData(GTE_DATA_MAC1 + i));
    PE_GTE_WriteCtrl(GTE_CTRL_ROT_11, c0);
    PE_GTE_WriteCtrl(GTE_CTRL_ROT_22, c2);
    PE_GTE_WriteCtrl(GTE_CTRL_ROT_33, c4);
}

int32_t pe_plat_gte_rot_trans_pers(uint32_t v0, uint32_t sxy_out, uint32_t p, uint32_t flag)
{
    load_vertex(0, v0);
    PE_GTE_Command(GTE_CMD_RTPS);
    PE_StoreU32(sxy_out, PE_GTE_ReadData(GTE_DATA_SXY2));
    PE_StoreU32(p, PE_GTE_ReadData(GTE_DATA_IR0));
    PE_StoreU32(flag, PE_GTE_ReadCtrl(GTE_CTRL_FLAG));
    return (int32_t)(PE_GTE_ReadData(GTE_DATA_SZ3) >> 2);
}

int32_t pe_plat_gte_rot_trans_pers3(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t sxy0,
                                    uint32_t sxy1, uint32_t sxy2, uint32_t p, uint32_t flag)
{
    load_vertex(0, v0);
    load_vertex(1, v1);
    load_vertex(2, v2);
    PE_GTE_Command(GTE_CMD_RTPT);
    PE_StoreU32(sxy0, sxy(0));
    PE_StoreU32(sxy1, sxy(1));
    PE_StoreU32(sxy2, sxy(2));
    PE_StoreU32(p, PE_GTE_ReadData(GTE_DATA_IR0));
    PE_StoreU32(flag, PE_GTE_ReadCtrl(GTE_CTRL_FLAG));
    return (int32_t)(PE_GTE_ReadData(GTE_DATA_SZ3) >> 2);
}

uint32_t pe_plat_gte_rot_trans(uint32_t v0, uint32_t out, uint32_t flag)
{
    uint32_t f;
    unsigned i;
    load_vertex(0, v0);
    PE_GTE_Command(GTE_CMD_MVMVA_R_V0_TR);
    for (i = 0; i < 3u; i++)
        PE_StoreU32(out + 4u * i, PE_GTE_ReadData(GTE_DATA_MAC1 + i));
    f = PE_GTE_ReadCtrl(GTE_CTRL_FLAG);
    PE_StoreU32(flag, f);
    return f;
}

/* Back-face culled: the screen coordinates, depth and average Z are only
 * delivered when the NCLIP result is positive. */
int32_t pe_plat_gte_rot_average_nclip3(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t sxy0,
                                       uint32_t sxy1, uint32_t sxy2, uint32_t p, uint32_t otz,
                                       uint32_t flag)
{
    int32_t area;
    load_vertex(0, v0);
    load_vertex(1, v1);
    load_vertex(2, v2);
    PE_GTE_Command(GTE_CMD_RTPT);
    PE_StoreU32(flag, PE_GTE_ReadCtrl(GTE_CTRL_FLAG));
    PE_GTE_Command(GTE_CMD_NCLIP);
    area = (int32_t)PE_GTE_ReadData(GTE_DATA_MAC0);
    if (area > 0) {
        PE_StoreU32(sxy0, sxy(0));
        PE_StoreU32(sxy1, sxy(1));
        PE_StoreU32(sxy2, sxy(2));
        PE_StoreU32(p, PE_GTE_ReadData(GTE_DATA_IR0));
        PE_GTE_Command(GTE_CMD_AVSZ3);
        PE_StoreU32(otz, PE_GTE_ReadData(GTE_DATA_OTZ));
    }
    return area;
}

int32_t pe_plat_gte_rot_average_nclip4(uint32_t v0, uint32_t v1, uint32_t v2, uint32_t v3,
                                       uint32_t sxy0, uint32_t sxy1, uint32_t sxy2, uint32_t sxy3,
                                       uint32_t p, uint32_t otz, uint32_t flag)
{
    int32_t area;
    uint32_t f;
    load_vertex(0, v0);
    load_vertex(1, v1);
    load_vertex(2, v2);
    PE_GTE_Command(GTE_CMD_RTPT);
    f = PE_GTE_ReadCtrl(GTE_CTRL_FLAG);
    PE_StoreU32(flag, f);
    PE_GTE_Command(GTE_CMD_NCLIP);
    area = (int32_t)PE_GTE_ReadData(GTE_DATA_MAC0);
    if (area > 0) {
        PE_StoreU32(sxy0, sxy(0));
        PE_StoreU32(sxy1, sxy(1));
        PE_StoreU32(sxy2, sxy(2));
        load_vertex(0, v3);
        PE_GTE_Command(GTE_CMD_RTPS);
        PE_StoreU32(sxy3, sxy(2));
        PE_StoreU32(p, PE_GTE_ReadData(GTE_DATA_IR0));
        PE_StoreU32(flag, f | PE_GTE_ReadCtrl(GTE_CTRL_FLAG));
        PE_GTE_Command(GTE_CMD_AVSZ4);
        PE_StoreU32(otz, PE_GTE_ReadData(GTE_DATA_OTZ));
    }
    return area;
}
