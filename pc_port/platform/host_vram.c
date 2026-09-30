/*
 * Read-only projection of the single PE_GPU VRAM authority.
 *
 * This file deliberately has no guest-memory or GPU-write API.  It exposes
 * the exact 1024x512 words already produced by translated retail LoadImage
 * work as either little-endian raw data or a diagnostic RGB555 PPM.
 */

#include "host_vram.h"

#include <stdio.h>

static uint8_t Expand5(uint16_t value)
{
    value &= 0x1Fu;
    return (uint8_t)((value << 3) | (value >> 2));
}

void HostVRAM_DecodePixel(uint16_t pixel, uint8_t rgb[3])
{
    if (rgb == NULL) {
        return;
    }
    rgb[0] = Expand5(pixel);
    rgb[1] = Expand5(pixel >> 5);
    rgb[2] = Expand5(pixel >> 10);
}

int HostVRAM_CopyRGB(uint8_t *dst, size_t dst_size)
{
    uint32_t x;
    uint32_t y;

    if (dst == NULL || dst_size < HOST_VRAM_RGB_BYTES) {
        return -1;
    }

    for (y = 0; y < PE_GPU_VRAM_HEIGHT; y++) {
        for (x = 0; x < PE_GPU_VRAM_WIDTH; x++) {
            uint16_t pixel;
            size_t offset = ((size_t)y * PE_GPU_VRAM_WIDTH + x) * 3u;

            if (!PE_GPU_ReadVRAM(x, y, &pixel)) {
                return -1;
            }
            HostVRAM_DecodePixel(pixel, dst + offset);
        }
    }
    return 0;
}

int HostVRAM_WriteRaw(const char *path)
{
    FILE *file;
    uint32_t x;
    uint32_t y;
    uint8_t row[PE_GPU_VRAM_WIDTH * 2u];
    int result = 0;

    if (path == NULL || (file = fopen(path, "wb")) == NULL) {
        return -1;
    }

    for (y = 0; y < PE_GPU_VRAM_HEIGHT && result == 0; y++) {
        for (x = 0; x < PE_GPU_VRAM_WIDTH; x++) {
            uint16_t pixel;

            if (!PE_GPU_ReadVRAM(x, y, &pixel)) {
                result = -1;
                break;
            }
            row[x * 2u] = (uint8_t)pixel;
            row[x * 2u + 1u] = (uint8_t)(pixel >> 8);
        }
        if (result == 0 && fwrite(row, 1u, sizeof(row), file) != sizeof(row)) {
            result = -1;
        }
    }
    if (fclose(file) != 0) {
        result = -1;
    }
    return result;
}

int HostVRAM_WritePPM(const char *path)
{
    FILE *file;
    uint32_t x;
    uint32_t y;
    uint8_t row[PE_GPU_VRAM_WIDTH * 3u];
    int result = 0;

    if (path == NULL || (file = fopen(path, "wb")) == NULL) {
        return -1;
    }
    if (fprintf(file, "P6\n%u %u\n255\n",
                PE_GPU_VRAM_WIDTH, PE_GPU_VRAM_HEIGHT) < 0) {
        result = -1;
    }

    for (y = 0; y < PE_GPU_VRAM_HEIGHT && result == 0; y++) {
        for (x = 0; x < PE_GPU_VRAM_WIDTH; x++) {
            uint16_t pixel;

            if (!PE_GPU_ReadVRAM(x, y, &pixel)) {
                result = -1;
                break;
            }
            HostVRAM_DecodePixel(pixel, row + x * 3u);
        }
        if (result == 0 && fwrite(row, 1u, sizeof(row), file) != sizeof(row)) {
            result = -1;
        }
    }
    if (fclose(file) != 0) {
        result = -1;
    }
    return result;
}
