/* Graphics-library debug-level setter (guest 0x80074BB8), written clean-room
 * from black-box vectors of the original code
 * (pc_port/tools/pe_cd_leaf_vectors.py cdgen12, checked by
 * pc_port/tools/pe_spu_caller_check.c), with no SDK source.
 * Behaviour is exactly the guest loads, stores and the one call through a
 * function-pointer word seen in the vectors. */
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"

#define ADDR_GFX_LOG_VECTOR  0x80095748u  /* 32-bit function-pointer word (the log vector) */
#define ADDR_GFX_AUX_A       0x8009574Cu  /* byte reported to the log vector as the third argument */
#define ADDR_GFX_DEBUG_LEVEL 0x8009574Eu  /* byte holding the current debug level */
#define ADDR_GFX_AUX_B       0x8009574Fu  /* byte reported to the log vector as the fourth argument */
#define ADDR_GFX_LEVEL_FMT   0x80011814u  /* guest address of the level-change message format */

/* Store the new level and return the previous one.  A non-zero level is also
 * reported through the log vector: (format, level, aux byte A, aux byte B),
 * the level being re-read from its byte after the store. */
unsigned char pe_plat_gpu_set_graph_debug(unsigned char level)
{
    unsigned char previous = PE_LoadU8(ADDR_GFX_DEBUG_LEVEL);

    PE_StoreU8(ADDR_GFX_DEBUG_LEVEL, level);
    if (level != 0) {
        uint32_t vector = PE_LoadU32(ADDR_GFX_LOG_VECTOR);
        unsigned char current = PE_LoadU8(ADDR_GFX_DEBUG_LEVEL);
        unsigned char aux_a = PE_LoadU8(ADDR_GFX_AUX_A);
        unsigned char aux_b = PE_LoadU8(ADDR_GFX_AUX_B);

        PE_GuestCall("@80074BB8:f", vector, 4,
                     (uintptr_t)ADDR_GFX_LEVEL_FMT, (uintptr_t)current,
                     (uintptr_t)aux_a, (uintptr_t)aux_b);
    }
    return previous;
}
