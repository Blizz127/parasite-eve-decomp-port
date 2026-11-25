/* libcard SIO acknowledge wait (guest 0x800834E8), written clean-room as a
 * native from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen21, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Behaviour is
 * exactly the guest loads, stores and timeout-check calls seen in the
 * vectors; no claim is made about any source. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_plat/pad.h"
#include "pe_plat/bios_file.h"
#include "game_port.h"

#define ADDR_PTR_ISTAT      0x8009B784u  /* word: address of the interrupt-status word */
#define ADDR_PTR_SIO        0x8009B788u  /* word: address of the SIO register block (re-read each pass) */

#define ISTAT_ACK_VALUE     0xFFFFFF7Fu  /* stored to the status word: clears bit 7 only */
#define SIO_STAT_OFF        0x04u        /* halfword: bit 7 set while waiting */
#define SIO_CTRL_OFF        0x0Au        /* halfword: bit 4 is set on success */
#define SIO_STAT_ACK_BIT    0x0080u
#define SIO_CTRL_ACK_SET    0x0010u

/* Port contracts (not behaviour of the original; the vectors never exercise
 * them and neither adds a guest load or store): (1) PE_Port_ShouldStop() is
 * checked at the top of each iteration of the bit-7 wait loop; a non-zero
 * result returns 0 (the function's own timeout result) at once, with no
 * further guest access.  (2) Right after that check,
 * pe_plat_sio_poll_tick() is called once per iteration: the port's clock is
 * deterministic CPU time that nothing else advances while native code spins,
 * so the 0x80084FE4 timeout could otherwise never arrive. */

/* 0x800834E8: store 0xFFFFFF7F through the word pointer at 0x8009B784, then
 * poll the halfword at (pointer at 0x8009B788) + 4.  While its bit 7 is set,
 * call the timeout check (func_80084FE4); a non-zero result returns 0.  When
 * bit 7 reads clear, read the halfword at (pointer at 0x8009B788) + 0xA, store
 * it back with bit 4 set, and return 1. */
int pe_plat_card_sio_ack_wait(void)
{
    uint32_t base;
    uint16_t ctrl;

    PE_StoreU32(PE_LoadU32(ADDR_PTR_ISTAT), ISTAT_ACK_VALUE);
    for (;;) {
        if (PE_Port_ShouldStop() != 0)
            return 0;
        pe_plat_sio_poll_tick();
        base = PE_LoadU32(ADDR_PTR_SIO);
        if ((PE_LoadU16(base + SIO_STAT_OFF) & SIO_STAT_ACK_BIT) == 0u)
            break;
        if (func_80084FE4() != 0)
            return 0;
    }
    base = PE_LoadU32(ADDR_PTR_SIO);
    ctrl = PE_LoadU16(base + SIO_CTRL_OFF);
    PE_StoreU16(base + SIO_CTRL_OFF, (uint16_t)(ctrl | SIO_CTRL_ACK_SET));
    return 1;
}
