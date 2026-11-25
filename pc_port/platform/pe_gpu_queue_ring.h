/* Standalone queue word reader; not yet bound to the production pump. */
#ifndef PE_GPU_QUEUE_RING_H
#define PE_GPU_QUEUE_RING_H
#include <stdint.h>

/* Reads one documented entry field after 32-bit guest arithmetic and range
 * validation. On failure, returns 0 and leaves *out unchanged; stop policy
 * remains the caller's responsibility. */
int PE_GpuQueue_ReadWord(uint32_t consumer, uint32_t field, uint32_t *out);
#endif
