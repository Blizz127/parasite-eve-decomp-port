/* GTE VectorNormal, batch 4 (guest 0x80078134), written clean-room from
 * black-box vectors of the original code
 * (pc_port/tools/pe_gte_cmd_vectors.py batch4), with no SDK source.
 * Evidence: docs/evidence/native-gte-vn-20261031/.
 *
 * Reads a three-word vector (x, y, z; each word's low 16 bits are the
 * component), writes its unit vector in 4.12 fixed point (three words) and
 * returns the squared length.  The squared length is summed with trapping
 * signed adds, so a sum of 0x80000000 or more raises the console exception;
 * the port models that as a stop request (PE_PORT_STOP_UNRESOLVED_BOUNDARY)
 * followed by an immediate return, before anything is stored or any table is
 * read.  The reciprocal square root comes from a 16-bit table in the game's
 * data, indexed by the top byte of the squared length once it has been
 * normalised by an even shift.  A zero length is not special-cased: its top
 * byte is 0, so the table is read one 0x40-entry run below its start. */
#include <stdint.h>
#include "pe_plat/gte.h"
#include "pe_guest_ram.h"
#include "pe_sdk.h"
#include "game_port.h"

#define GTE_DATA_IR0        8u  /* scale factor for the GPF command */
#define GTE_DATA_IR1        9u
#define GTE_DATA_IR2        10u
#define GTE_DATA_IR3        11u
#define GTE_DATA_MAC1       25u
#define GTE_DATA_MAC2       26u
#define GTE_DATA_MAC3       27u
#define GTE_DATA_LZCS       30u /* leading-zero count source */
#define GTE_DATA_LZCR       31u /* leading-zero count result */

#define GTE_CMD_SQR         0x00A00428u /* MAC = IR * IR, lm=1, sf=0 */
#define GTE_CMD_GPF         0x0190003Du /* MAC = IR * IR0, sf=0 */

#define RSQRT_TABLE         0x80096250u /* u16 per top byte 0x40..0xFF */
#define RSQRT_FIRST_TOP     0x40u
#define LZ_BITS             32u
#define RESULT_SHIFT        15u /* table scale: unit length is 4096 * 8 / sqrt(top) */
#define SUM_TRAP_LIMIT      0x80000000u /* first sum a signed add rejects */

static void raise_trap(void)
{
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

uint32_t pe_plat_gte_vector_normal(uint32_t in, uint32_t out)
{
    uint32_t v[3], mac[3], sum, lz, shift, top, scale, i;

    for (i = 0; i < 3; i++)
        v[i] = PE_LoadU32(in + 4u * i);

    for (i = 0; i < 3; i++)
        PE_GTE_WriteData(GTE_DATA_IR1 + i, v[i]);
    PE_GTE_Command(GTE_CMD_SQR);
    mac[0] = PE_GTE_ReadData(GTE_DATA_MAC1);
    mac[1] = PE_GTE_ReadData(GTE_DATA_MAC2);
    mac[2] = PE_GTE_ReadData(GTE_DATA_MAC3);

    /* Squares are non-negative, so any partial sum overflows only if the
     * total does. */
    sum = mac[0] + mac[1] + mac[2];
    if (sum >= SUM_TRAP_LIMIT) {
        raise_trap();
        return 0;
    }

    PE_GTE_WriteData(GTE_DATA_LZCS, sum);
    lz = PE_GTE_ReadData(GTE_DATA_LZCR);
    shift = lz & ~1u;
    /* A zero sum has lz 32; its top byte is 0 whichever way a shift by 32 resolves. */
    top = (shift < LZ_BITS ? sum << shift : sum) >> 24;
    scale = PE_LoadU16(RSQRT_TABLE + 2u * (top - RSQRT_FIRST_TOP));

    PE_GTE_WriteData(GTE_DATA_IR0, scale);
    for (i = 0; i < 3; i++)
        PE_GTE_WriteData(GTE_DATA_IR1 + i, v[i]);
    PE_GTE_Command(GTE_CMD_GPF);
    mac[0] = PE_GTE_ReadData(GTE_DATA_MAC1);
    mac[1] = PE_GTE_ReadData(GTE_DATA_MAC2);
    mac[2] = PE_GTE_ReadData(GTE_DATA_MAC3);

    /* The table assumes the even-shifted length; undo half the shift here. */
    for (i = 0; i < 3; i++) {
        uint32_t down = RESULT_SHIFT - (lz >> 1);
        int32_t m = (int32_t)mac[i];
        PE_StoreU32(out + 4u * i, (uint32_t)(m >> (down & 31u)));
    }
    return sum;
}
