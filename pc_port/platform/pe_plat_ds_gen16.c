/* libds text routine 0x80081D18 (start a read and wait for it), written
 * clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen16, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.  Its behaviour is
 * exactly the calls seen in the vectors; it performs no guest loads or
 * stores of its own. */
#include <stdint.h>
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "game_port.h"

#include "pe_decomp_boundary.h"   /* PE_Decomp_BoundaryFrom */

#define ADDR_DS_READ_START   0x80080E34u /* read-start entry (no port implementation) */
#define DS_READ_START_ARITY  4u          /* arguments passed to the read-start entry */
#define DS_READ_START_MODE   0x80u       /* constant fourth argument of the read-start entry */
#define DS_POLL_ARG          0u          /* constant argument of every poll call */
#define DS_POS_RECORD_BYTES  8u          /* position record taken from the guest frame */
#define DS_RESULT_DONE       1            /* returned when the final poll result is 0 */
#define DS_RESULT_FAILED     0            /* returned when the final poll result is negative */

/* Guest 0x80081D18: convert a1 into a position record in a frame buffer
 * (func_80080B44(a1, record)), start the read through 0x80080E34 with
 * (record, a0, a2, 0x80), then poll func_800811E4(0) until it stops
 * returning a positive value.  The result is 1 when the final poll value is
 * 0 and 0 when it is negative.  Port contract: PE_Port_ShouldStop() is
 * checked before every poll; if it is nonzero, polling stops and the result
 * is 0.  The frame buffer stays live until the function returns. */
int pe_plat_ds_text_904(int a0, int a1, int a2)
{
    pe_addr_t record = PE_GuestFrame_Enter(DS_POS_RECORD_BYTES);
    int poll;
    int result;

    (void)func_80080B44((int32_t)a1, record);
    (void)PE_Decomp_BoundaryFrom("func_80081D18", "func_80080E34", ADDR_DS_READ_START,
                                 DS_READ_START_ARITY, (uintptr_t)record,
                                 (uintptr_t)(uint32_t)a0, (uintptr_t)(uint32_t)a2,
                                 DS_READ_START_MODE);

    for (;;) {
        if (PE_Port_ShouldStop() != 0) {
            result = DS_RESULT_FAILED;
            break;
        }
        poll = func_800811E4(DS_POLL_ARG);
        if (poll > 0)
            continue;
        result = (poll == 0) ? DS_RESULT_DONE : DS_RESULT_FAILED;
        break;
    }
    PE_GuestFrame_Leave(record, DS_POS_RECORD_BYTES);
    return result;
}
