/* libds DsDataCallback (guest 0x800824F0) and libds DsSystemStatus
 * (0x8007F72C), written clean-room as natives from
 * black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * cdgen33, checked by pc_port/tools/pe_spu_caller_check.c with
 * -DCHK_HAVE_CD_GEN33).  Independent re-implementation: the behaviour below is
 * exactly the calls, loads and stores seen in the vectors (hashes in
 * docs/evidence/native-cd-rows-20261125/REPORT.md); no SDK source and nothing
 * from any other implementation was used, and no claim is made about the
 * original's source.  The original code was only run as a black box.
 *
 * 0x800824F0 (one argument): calls 0x80073CF4 with (3, argument) and returns
 * its result unchanged.  It has no memory accesses of its own.
 *
 * 0x8007F72C (no arguments): pe_plat_sio_poll_tick() once (host contract, below), then r = 0x8007FBF0(0).  When r == 1 (the full
 * word) it calls 0x8007F778() and returns 2 if that result is positive as a
 * signed word, else 1.  Any other r is returned unchanged and 0x8007F778 is
 * not called.  Vectors: low-byte / low-half / bit-0 look-alikes of 1 do not
 * take the second arm; a zero, negative or 0x80000000 queue length gives 1; a
 * positive one with a zero low half gives 2.
 *
 *
 * 0x8007C13C (no arguments, no result): loads the CD register pointer word
 * 0x8009B27C and reads the byte it points at (the CD index/status register)
 * before anything else, then loops: r = 0x8007AAB4(); stop when r == 0; when
 * (r & 4) the callback word 0x8009AFB8 is loaded and, if nonzero, called with
 * (byte at 0x8009B295, 0x800A3468); when (r & 2) the callback word 0x8009AFB4
 * is loaded and, if nonzero, called with (byte at 0x8009B294, 0x800A3460).
 * The bit-4 callback comes first.  The status byte is loaded only when the
 * word is nonzero.  The register pointer word is reloaded after the loop and
 * the byte read before the loop, masked with 3, is stored through it.  Tests
 * are on the full result and full callback word (vectors: 0x100, 0x10000,
 * bit 31 and the KUSEG alias of a sentinel are not look-alikes).  Callbacks
 * go through the address-keyed dispatcher PE_Cd_DispatchDataCallback, which
 * resolves guest callback addresses for the whole libcd/libds poll family.
 * Undetermined: the vectors cannot show whether the byte is read before or
 * after the first 0x8007AAB4 call except through the callee's register write
 * (vector model: the first call writes the index byte, and the restored value
 * follows the pre-loop read).
 *
 * Host contracts (not behaviour of the original; the vectors never exercise
 * them):
 *  - 0x8007F72C: guest code polls it in wait loops (the status lane stays 2
 *    until the device model advances), and the port's clock only moves when
 *    something ticks it, so each call stands for one guest poll iteration:
 *    pe_plat_sio_poll_tick() (advances the CD device / host clock; no guest
 *    access).  Without it the native CD init wait spins forever.
 *  - 0x8007C13C: PE_Port_ShouldStop() is checked after each 0x8007AAB4 call
 *    (non-zero leaves the loop), pe_plat_sio_poll_tick() runs once per loop
 *    pass, and after 0x40000 passes PE_Port_RequestStop(
 *    PE_PORT_STOP_UNRESOLVED_BOUNDARY) is called and the loop is left.  In
 *    every case the function finishes with its normal restore. */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_cdreg.h"
#include "pe_plat/cd.h"
#include "pe_plat/irq_callback.h"
#include "pe_plat/bios_file.h"
#include "game_port.h"
#include "pe_sdk.h"

#define POLL_BUDGET   0x40000u
#define PTR_REGISTER  0x8009B27Cu  /* CD index/status register pointer */
#define CB_BIT2       0x8009AFB4u  /* callback word for result bit 1 (value 2) */
#define CB_BIT4       0x8009AFB8u  /* callback word for result bit 2 (value 4) */
#define STATUS_BIT2   0x8009B294u
#define STATUS_BIT4   0x8009B295u
#define RESPONSE_BIT2 0x800A3460u
#define RESPONSE_BIT4 0x800A3468u

uint32_t pe_plat_ds_data_callback(uint32_t callback)
{
    return func_80073CF4(3u, callback);
}

int pe_plat_ds_system_status(void)
{
    int lane;

    pe_plat_sio_poll_tick();
    lane = func_8007FBF0(0);
    if (lane != 1) return lane;
    return func_8007F778() > 0 ? 2 : 1;
}


void pe_plat_cd_intr_poll(void)
{
    uint32_t reg, bits, cb, passes = 0u;
    uint8_t index;

    reg = PE_LoadU32(PTR_REGISTER);
    index = PE_CdLoadU8(reg);
    for (;;) {
        bits = (uint32_t)func_8007AAB4();
        if (PE_Port_ShouldStop()) break;
        if (!bits) break;
        if (passes++ >= POLL_BUDGET) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            break;
        }
        pe_plat_sio_poll_tick();
        if (bits & 4u) {
            cb = PE_LoadU32(CB_BIT4);
            if (cb) PE_Cd_DispatchDataCallback(cb, PE_LoadU8(STATUS_BIT4), RESPONSE_BIT4);
        }
        if (bits & 2u) {
            cb = PE_LoadU32(CB_BIT2);
            if (cb) PE_Cd_DispatchDataCallback(cb, PE_LoadU8(STATUS_BIT2), RESPONSE_BIT2);
        }
    }
    reg = PE_LoadU32(PTR_REGISTER);
    PE_CdStoreU8(reg, (uint8_t)(index & 3u));
}
