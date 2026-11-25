/* libds DsGetDiskType (guest 0x80082314), written clean-room as a native from
 * black-box vectors of the original code (pc_port/tools/pe_cd_leaf_vectors.py
 * cdgen38, checked by pc_port/tools/pe_spu_caller_check.c with
 * -DCHK_HAVE_DS_GEN38).  Independent re-implementation: the behaviour below is
 * exactly the calls, loads and stores seen in the vectors (hashes in
 * docs/evidence/native-cd-rows-20261125/REPORT.md); no SDK source and nothing
 * from any other implementation was used, and no claim is made about the
 * original's source.  The original code was only run as a black box.
 *
 * Behaviour (no arguments; the result is the int the vectors show):
 *  1. if 0x8007FBF0(0) == 2 (full word) call 0x8007FBF0(1); when that is
 *     exactly 0x10 return 0x10 at once (nothing else called, no store);
 *     other values, including 0x110, 0x10010 and 0x80000010, are not.
 *  2. loop: if 0x8007F72C() == 1 leave the loop for step 3; if the next
 *     0x8007F72C() == 3 return 1 (no store, nothing else called).  (Vectors
 *     over every X/1/3 prefix to length 5 fit this alternation exactly: the
 *     odd-numbered call tests for 1, the even-numbered one for 3; other
 *     values, including 0x101, 0x10001, 0x80000000 and -1, test as neither.)
 *  3. if 0x800822AC() != 0 call 0x80081DF8().  Build a position record in the
 *     caller's own frame: 0x80080B44(0x10, record); store 0 to the word
 *     0x800B28F8 (after that call, before the next); r = 0x8007F0C8(0x20,
 *     record, 0x1B, 0x80082400, 0).  If r == 0 (full word) return 2.
 *     Otherwise poll the word 0x800B28F8 until it is nonzero (full word),
 *     then load it once more and return that value.  (0x80082400 is the
 *     address of the command-complete callback text_EC, which stores 2 to
 *     the word; the callback address goes out as a plain integer.)
 *
 * Host contracts (not behaviour of the original; the vectors never exercise
 * them, and vectors that never end the wait are not generated):
 *  - 0x8007F72C is itself the per-poll tick (pe_plat_cd_gen33.c), so the
 *    step-2 loop needs none; PE_Port_ShouldStop() is checked on each pass and
 *    a pass budget of 0x40000 calls PE_Port_RequestStop(
 *    PE_PORT_STOP_UNRESOLVED_BOUNDARY); both leave the loop and return 1.
 *  - Step 3's word wait calls pe_plat_sio_poll_tick() once per pass (it
 *    advances the CD device / host clock, which runs the command callback that
 *    writes the word), checks PE_Port_ShouldStop() before each re-read and
 *    uses the same 0x40000 budget with PE_Port_RequestStop; after either, the
 *    function returns the word as loaded (normal path). */
#include <stdint.h>

#include "pe_guest_ram.h"
#include "pe_plat/cd.h"
#include "pe_plat/bios_file.h"
#include "game_port.h"
#include "pe_sdk.h"

#define WORD_DISK_TYPE  0x800B28F8u
#define CALLBACK_TEXT_EC 0x80082400u
#define POLL_BUDGET     0x40000u
#define RECORD_BYTES    8u

int pe_plat_ds_get_disk_type(void)
{
    uint32_t passes = 0u, word;
    pe_addr_t record;
    int r;

    if (func_8007FBF0(0) == 2 && func_8007FBF0(1) == 0x10) return 0x10;
    for (;;) {
        if (func_8007F72C() == 1) break;
        if (PE_Port_ShouldStop()) return 1;
        if (func_8007F72C() == 3) return 1;
        if (PE_Port_ShouldStop()) return 1;
        if (passes++ >= POLL_BUDGET) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 1;
        }
    }
    if (func_800822AC() != 0) func_80081DF8();
    record = PE_GuestFrame_Enter(RECORD_BYTES);
    (void)func_80080B44(0x10, record);
    PE_StoreU32(WORD_DISK_TYPE, 0u);
    r = func_8007F0C8(0x20u, record, 0x1B, CALLBACK_TEXT_EC, 0u);
    PE_GuestFrame_Leave(record, RECORD_BYTES);
    if (r == 0) return 2;
    passes = 0u;
    while ((word = PE_LoadU32(WORD_DISK_TYPE)) == 0u) {
        if (PE_Port_ShouldStop()) break;
        if (passes++ >= POLL_BUDGET) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            break;
        }
        pe_plat_sio_poll_tick();
    }
    return (int)PE_LoadU32(WORD_DISK_TYPE);
}
