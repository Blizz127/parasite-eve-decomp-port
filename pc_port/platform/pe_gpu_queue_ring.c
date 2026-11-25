/* Independent queue-field reader derived from the documented guest layout. */
#include "pe_gpu_queue_ring.h"
#include "pe_guest_ram.h"

int PE_GpuQueue_ReadWord(uint32_t consumer, uint32_t field, uint32_t *out)
{
    if (!out || (field != 0u && field != 4u && field != 8u)) return 0;
    /* The original performs raw 32-bit multiplication/addition; do not mask
     * the consumer to 63 before validating the resulting guest address. */
    uint32_t address = 0x800bd030u + consumer * 0x60u + field;
    if (!PE_RangeIsRam(address, sizeof(uint32_t))) return 0;
    *out = PE_LoadU32(address);
    return 1;
}
