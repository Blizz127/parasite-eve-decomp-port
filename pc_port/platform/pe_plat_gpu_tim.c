/* pe_plat_gpu_load_tim (guest 0x800718D0): upload a TIM image's pixel and
 * palette blocks to VRAM.
 *
 * Provenance: written clean-room from black-box vectors of the original code
 * (pc_port/tools/pe_gpu_tim_vectors.py) and the psx-spx TIM documentation,
 * with no SDK source and without reading the previous provider.
 *
 * TIM layout (psx-spx): +0 id word, +4 flags (bit 3 = CLUT block present),
 * +8 first block: size word (bytes, counting itself), RECT {x,y,w,h} halfwords
 * at +4, data at +12.  With a CLUT the pixel block follows at (+8 + size).
 * Vectors show: the pixel block is uploaded first, then the palette; the
 * size word is read only when the CLUT bit is set; the result is the pixel
 * data address.  Address arithmetic is 32-bit wrapping, no masking.
 */
#include <stdint.h>
#include "psx_compat.h"
#include "pe_guest_ram.h"

static int upload_block(uint32_t block, uint32_t *data)
{
    RECT r;
    r.x = (int16_t)PE_LoadU16(block + 4);
    r.y = (int16_t)PE_LoadU16(block + 6);
    r.w = (int16_t)PE_LoadU16(block + 8);
    r.h = (int16_t)PE_LoadU16(block + 10);
    *data = block + 12;
    return pe_plat_gpu_load_rect_host(&r, block + 12);
}

uint32_t pe_plat_gpu_load_tim(uint32_t tim)
{
    uint32_t flags = PE_LoadU32(tim + 4);
    uint32_t block = tim + 8, pix, d;
#ifdef PE_TIM_MUTANT_NOCLUT
    flags = 0;
#endif
    if (flags & 8u) {
        uint32_t size = PE_LoadU32(block);
        pix = block + size;
#ifdef PE_TIM_MUTANT_SWAP
        upload_block(block, &d);
        upload_block(pix, &d);
        return d;
#endif
        upload_block(pix, &d);
        upload_block(block, &(uint32_t){0});
        return d;
    }
    upload_block(block, &d);
    return d;
}
