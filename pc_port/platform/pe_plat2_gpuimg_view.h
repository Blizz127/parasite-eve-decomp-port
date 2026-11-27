/* Private to the gpuimg providers: a rectangle that is either guest memory (read with the guest loads) or a host
 * object (a caller's stack RECT, read in place). */
#ifndef PE_PLAT2_GPUIMG_VIEW_H
#define PE_PLAT2_GPUIMG_VIEW_H
#include <stdint.h>
#include <string.h>
#include "pe_guest_ram.h"
#include "pe_plat/gpuimg.h"

typedef struct { pe_addr_t guest; const RECT *host; } GpuImgRect;

/* A host pointer inside the guest RAM or scratchpad allocation maps to its guest address; any other pointer has no
 * guest address.  Returns 1 and sets *guest when it has one. */
static inline int gpuimg_guest_address(const void *host, pe_addr_t *guest)
{
    uintptr_t p = (uintptr_t)host, ram = (uintptr_t)PE_Translate(PE_RAM_BASE, 1);
    if (host == NULL) { *guest = 0u; return 1; }
    if (p >= ram && p - ram < PE_RAM_SIZE) { *guest = PE_RAM_BASE + (pe_addr_t)(p - ram); return 1; }
    {
        uintptr_t sp = (uintptr_t)PE_Translate(PE_SCRATCHPAD_BASE, 1);
        if (p >= sp && p - sp < PE_SCRATCHPAD_SIZE) { *guest = PE_SCRATCHPAD_BASE + (pe_addr_t)(p - sp); return 1; }
    }
    return 0;
}
static inline GpuImgRect gpuimg_view(const RECT *rect)
{
    GpuImgRect v;
    v.guest = 0u; v.host = rect;
    if (gpuimg_guest_address(rect, &v.guest)) v.host = NULL;
    return v;
}
static inline uint32_t gpuimg_rd16(const GpuImgRect *v, uint32_t off)
{
    if (v->host) { uint16_t h; memcpy(&h, (const uint8_t *)v->host + off, 2); return h; }
    return PE_LoadU16(v->guest + off);
}
static inline uint32_t gpuimg_rd32(const GpuImgRect *v, uint32_t off)
{
    if (v->host) { uint32_t w; memcpy(&w, (const uint8_t *)v->host + off, 4); return w; }
    return PE_LoadU32(v->guest + off);
}
#endif
