/* libpad root-counter-2 wait pair: guest 0x80084FC4 (record the start) and
 * 0x80084FE4 (has the wait elapsed?).  Written clean-room from black-box
 * vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py,
 * entries 0x80084FC4/0x80084FE4), with no SDK source.
 * Evidence: docs/evidence/native-rcnt2-20261031/. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"

/* Root counter 2 hardware registers (psx-spx). */
#define RC2_COUNTER_REG   0x1F801120u  /* 16-bit counter value */
#define RC2_MODE_REG      0x1F801124u  /* 16-bit counter mode */
#define RC2_TARGET_REG    0x1F801128u  /* 16-bit counter target */

/* Mode bit 9: set = counter runs on the divided (system clock / 8) source,
 * and the elapsed count is compared with the limit as is; clear = system
 * clock, and the elapsed count is divided by 8 first. */
#define RC2_MODE_DIV8_BIT 0x0200u
#define RC2_CLOCK_SHIFT   3u           /* divide-by-8 scaling */

/* Counter turn length used on wrap when the target register reads 0. */
#define RC2_WRAP_DEFAULT  0x10000u

/* Guest words: the wait limit (stored from the argument) and the counter
 * value sampled when the wait was started. */
#define PAD_WAIT_LIMIT    0x800BD02Cu
#define PAD_WAIT_START    0x800A76D0u

/* Start a wait: store the limit, then sample the counter as the start. */
void pe_plat_pad_rc2_wait_set(uint32_t limit)
{
    PE_StoreU32(PAD_WAIT_LIMIT, limit);
    PE_StoreU32(PAD_WAIT_START, PE_LoadU16(RC2_COUNTER_REG));
}

/* Return 1 when the wait has run its limit, else 0.  Elapsed is the counter
 * minus the start (unsigned 32-bit).  When the counter is below the start
 * (it wrapped) the target register is read: a zero target gives a turn of
 * 0x10000, a nonzero target is read a second time and used as the turn
 * length; elapsed = counter + turn - start.  With mode bit 9 clear the
 * elapsed count is shifted right by 3 before the unsigned compare
 * "elapsed >= limit". */
int pe_plat_pad_rc2_wait_check(void)
{
    uint32_t now = PE_LoadU16(RC2_COUNTER_REG);
    uint32_t start = PE_LoadU32(PAD_WAIT_START);
    uint32_t elapsed, limit;
    uint32_t mode;

    if (now < start) {
        uint32_t turn = RC2_WRAP_DEFAULT;
        if (PE_LoadU16(RC2_TARGET_REG) != 0)
            turn = PE_LoadU16(RC2_TARGET_REG);
        elapsed = now + turn - start;
    } else {
        elapsed = now - start;
    }
    mode = PE_LoadU16(RC2_MODE_REG);
    limit = PE_LoadU32(PAD_WAIT_LIMIT);
    if (!(mode & RC2_MODE_DIV8_BIT))
        elapsed >>= RC2_CLOCK_SHIFT;
    return elapsed >= limit;
}
