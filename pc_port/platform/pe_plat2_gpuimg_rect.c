/* Rectangle report (guest 0x80074E28), written clean-room from the black-box vector sets gpu2b
 * (docs/evidence/native-gpu2-20261126), the port's host GPU/DMA model API and psx-spx; no SDK source, decompiled
 * source, tables or disassembly were read.
 *
 * Contract (every line traced to the vectors):
 *  - the debug level is the byte at 0x8009574E.  Level 2 always prints; level 1 prints only for an invalid
 *    rectangle; any other level (0, 3.. ) returns with no further access.
 *  - printing = two calls through the function pointer word 0x80095748 (a printf-like routine): first
 *    (label format 0x800118B8 at level 2, 0x80011898 at level 1, name), then (0x800118A4, x, y, w, h) with the four
 *    halfwords sign-extended.  The report returns nothing; the callers ignore the print result.
 *  - validity (level 1), with the halfword limits at 0x80095750 (horizontal) and 0x80095752 (vertical), checked
 *    in this order and stopping at the first failure: w > limit_h; x + w > limit_h; y > limit_v;
 *    y + h > limit_v; then any of x, y, w, h negative.  The vectors never show w == 0 or h == 0 with an
 *    otherwise valid rectangle, so zero sizes are treated as valid (undefined edge, see AUTHOR-REPORT). */
#include <stdint.h>
#include "pe_guestcode.h"
#include "pe_plat2_gpuimg_view.h"

#define W_LEVEL   0x8009574Eu
#define W_PRINTF  0x80095748u
#define LIM_H     0x80095750u
#define LIM_V     0x80095752u
#define FMT_LEVEL1 0x80011898u
#define FMT_LEVEL2 0x800118B8u
#define FMT_VALUES 0x800118A4u

static int16_t s16(uint32_t v) { return (int16_t)(uint16_t)v; }

static int rect_invalid(const GpuImgRect *r)
{
    int32_t lim_h = s16(PE_LoadU16(LIM_H)), w = s16(gpuimg_rd16(r, 4)), x, y, h, lim_v;
    if (w > lim_h) return 1;
    x = s16(gpuimg_rd16(r, 0));
    if (x + w > lim_h) return 1;
    y = s16(gpuimg_rd16(r, 2));
    lim_v = s16(PE_LoadU16(LIM_V));
    if (y > lim_v) return 1;
    h = s16(gpuimg_rd16(r, 6));
    if (y + h > lim_v) return 1;
    return x < 0 || y < 0 || w < 0 || h < 0;
}

void pe_plat_gpuimg_report_rect(pe_addr_t name, const RECT *rect)
{
    GpuImgRect r = gpuimg_view(rect);
    uint32_t level = PE_LoadU8(W_LEVEL), fmt;
    uintptr_t a[5];
    if (level == 1u) {
        if (!rect_invalid(&r)) return;
        fmt = FMT_LEVEL1;
    } else if (level == 2u) {
        fmt = FMT_LEVEL2;
    } else {
        return;
    }
    (void)PE_GuestCall("gpuimg_report_label", PE_LoadU32(W_PRINTF), 2, fmt, name, 0, 0);
    a[0] = FMT_VALUES;
    a[1] = (uintptr_t)(uint32_t)(int32_t)s16(gpuimg_rd16(&r, 0));
    a[2] = (uintptr_t)(uint32_t)(int32_t)s16(gpuimg_rd16(&r, 2));
    a[3] = (uintptr_t)(uint32_t)(int32_t)s16(gpuimg_rd16(&r, 4));
    a[4] = (uintptr_t)(uint32_t)(int32_t)s16(gpuimg_rd16(&r, 6));
    (void)PE_GuestCallN("gpuimg_report_values", PE_LoadU32(W_PRINTF), 5, a);
}
