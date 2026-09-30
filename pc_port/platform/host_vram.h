/* Read-only host diagnostics for the authoritative PSX VRAM image. */
#ifndef HOST_VRAM_H
#define HOST_VRAM_H

#include <stddef.h>
#include <stdint.h>

#include "pe_gpu.h"

#define HOST_VRAM_RGB_BYTES \
    ((size_t)PE_GPU_VRAM_WIDTH * (size_t)PE_GPU_VRAM_HEIGHT * 3u)
#define HOST_VRAM_RAW_BYTES \
    ((size_t)PE_GPU_VRAM_WIDTH * (size_t)PE_GPU_VRAM_HEIGHT * 2u)

/* Decode one PSX BGR555/STP word to RGB888.  STP is metadata and does not
 * alter the diagnostic color. */
void HostVRAM_DecodePixel(uint16_t pixel, uint8_t rgb[3]);

/* Copy the full current VRAM authority into a caller-owned RGB888 buffer.
 * This is a pure observation: no guest or GPU state is changed. */
int HostVRAM_CopyRGB(uint8_t *dst, size_t dst_size);

/* Portable host artifacts.  Raw words are always little-endian regardless
 * of host endianness; PPM is a 1024x512 P6 RGB888 image. */
int HostVRAM_WriteRaw(const char *path);
int HostVRAM_WritePPM(const char *path);

#endif /* HOST_VRAM_H */
