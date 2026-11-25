/* GTE helper leaves with divisions, batch 3 (guest 0x80077E64 SetFogNearFar /
 * 0x80078004 SquareRoot0 / 0x80079FB4 ratan2), written clean-room from
 * black-box vectors of the original code
 * (pc_port/tools/pe_gte_cmd_vectors.py batch3), with no SDK source.
 * Evidence: docs/evidence/native-gte-div-20261031/.
 *
 * The original guards its divisions: divide by zero and INT_MIN / -1 raise
 * exceptions on the console.  The port models an exception as a stop request
 * (PE_PORT_STOP_UNRESOLVED_BOUNDARY) followed by an immediate return; every
 * trap condition is tested before the C division that would fault on the host.
 * All wrapping arithmetic is done in uint32_t and converted afterwards. */
#include <stdint.h>
#include "pe_plat/gte.h"
#include "pe_guest_ram.h"
#include "pe_sdk.h"
#include "game_port.h"

#define GTE_CTRL_DQA        27u /* depth-cue slope, 16-bit */
#define GTE_CTRL_DQB        28u /* depth-cue base, 32-bit */
#define GTE_DATA_LZCS       30u /* leading-zero count source */
#define GTE_DATA_LZCR       31u /* leading-zero count result */

/* SetFogNearFar: a span (far - near) below this is rejected untouched. The
 * vectors only bound it to 96..103 (95 is rejected, 103 accepted); 100 is a
 * choice inside that range. */
#define FOG_MIN_SPAN        100

#define SQRT_POS_TABLE      0x800960BCu /* u16 per top byte 0x40..0xFF */
#define SQRT_NEG_TABLE      0x80095E3Cu /* u16 per top byte 0x00..0xFF */
#define RATAN_TABLE         0x8009A6ECu /* u16 per ratio 0..1024 (1/1024 steps) */
#define RATAN_BIG_DIVIDEND  0x200000 /* 2^21: where dividend << 10 overflows */
#define RATAN_RIGHT_ANGLE   1024        /* a quarter turn; full turn is 4096 */

static int32_t as_s32(uint32_t v)
{
    return (int32_t)v;
}

static void raise_trap(void)
{
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

/* Depth-cue constants so that the fog factor runs from 0 at `near_z` to
 * 0x1000 at `far_z` for a projection plane distance `h`.  The three divisions
 * truncate toward zero (C99), on 32-bit wrapped intermediates. */
void pe_plat_gte_set_fog_near_far(uint32_t near_z, uint32_t far_z, int32_t h)
{
    int32_t span = as_s32(far_z - near_z);
    int32_t slope_num, dqa;
    uint32_t base;

    if (span < FOG_MIN_SPAN)
        return;
    /* span is positive here, so neither wrapped division below can trap. */
    slope_num = as_s32(((0u - near_z) * far_z)) / span;
    slope_num = as_s32((uint32_t)slope_num << 8);
    if (h == 0 || (h == -1 && slope_num == INT32_MIN)) {
        raise_trap();
        return;
    }
    dqa = slope_num / h;
    if (dqa > 32767)
        dqa = 32767;
    else if (dqa < -32768)
        dqa = -32768;
    base = (uint32_t)(as_s32(far_z << 12) / span) << 12;
    PE_GTE_WriteCtrl(GTE_CTRL_DQA, (uint32_t)dqa);
    PE_GTE_WriteCtrl(GTE_CTRL_DQB, base);
}

/* Table square root: the value (as a signed word, so a leading run of ones
 * counts like leading zeros) is normalised by an even shift, its top byte
 * indexes a 16-bit table of 4.12 roots, and the root is scaled back down by
 * half the shift.  Zero and -1 return 0 without touching the tables. */
uint32_t pe_plat_gte_square_root0(uint32_t value)
{
    uint32_t lz, shift, top, entry;

    PE_GTE_WriteData(GTE_DATA_LZCS, value);
    lz = PE_GTE_ReadData(GTE_DATA_LZCR);
    if (lz >= 32u)
        return 0;
    shift = lz & ~1u;
    top = (value << shift) >> 24;
    if (value & 0x80000000u)
        entry = PE_LoadU16(SQRT_NEG_TABLE + 2u * top);
    else
        entry = PE_LoadU16(SQRT_POS_TABLE + 2u * (top - 0x40u));
    return (entry << 3) >> (shift / 2u);
}

/* Angle of (x, y) in 1/4096 turns; negative for y < 0.  The ratio of the
 * smaller magnitude to the larger one, in 1/1024 steps, indexes an arctangent
 * table covering one octant; the other octant is 1024 minus that.
 * Magnitudes use 32-bit negation, so INT_MIN stays negative: as a dividend it
 * counts as 0, and compares below every non-negative magnitude.  A dividend
 * of 2^21 or more would overflow the shift by 10, so the ratio is taken
 * against the divisor scaled down instead: dividend / (divisor >> 10). */
int32_t pe_plat_gte_ratan2(int32_t y, int32_t x)
{
    int32_t ay, ax, dividend, divisor;
    uint32_t idx, angle;
    int swapped;

    if (x == 0 && y == 0)
        return 0;
    ay = y < 0 ? as_s32(0u - (uint32_t)y) : y;
    ax = x < 0 ? as_s32(0u - (uint32_t)x) : x;
    swapped = !(ay < ax);
    dividend = swapped ? ax : ay;
    divisor = swapped ? ay : ax;
    if (divisor == 0) {
        raise_trap();
        return 0;
    }
    /* dividend >= 0 makes the divisor positive too (and >= 2048 when the
     * dividend reaches RATAN_BIG_DIVIDEND), so neither division can trap. */
    if (dividend < 0)
        idx = 0;
    else if (dividend >= RATAN_BIG_DIVIDEND)
        idx = (uint32_t)(dividend / (divisor >> 10));
    else
        idx = (uint32_t)((dividend << 10) / divisor);
    angle = PE_LoadU16(RATAN_TABLE + 2u * idx);
    if (swapped)
        angle = (uint32_t)RATAN_RIGHT_ANGLE - angle;
    if (x < 0)
        angle = 2u * (uint32_t)RATAN_RIGHT_ANGLE - angle;
    if (y < 0)
        angle = 0u - angle;
    return as_s32(angle);
}
