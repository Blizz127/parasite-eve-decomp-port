/* Address leaves of the libgpu / libgs object formats: clean-room provider
 * pc_port/platform/pe_plat2_gpuaddr.c, written from the black-box vector set
 * gpu2c (docs/evidence/native-gpu2-20261126) and psx-spx (GP0(E1h) texture
 * page attribute, CLUT attribute).  These bindings replace the rows of the
 * same guest names that gpu_address.h used to carry. */
#ifndef PE_PLAT_GPUADDR_H
#define PE_PLAT_GPUADDR_H
#include <stdint.h>

/* Texture-page word: page column, page row bits, blend mode, colour depth. */
int pe_plat_gpuaddr_texture_page(int tp, int abr, int x, int y);
/* Palette word: row in bits 6..15, column / 16 in bits 0..5. */
int pe_plat_gpuaddr_clut(int x, int y);
/* Object chunk locators: take the object base address and return the address
 * of an optional chunk (or 0), or the chunk that follows the optional parts. */
uint32_t pe_plat_gpuaddr_optional_chunk(uint32_t object);
uint32_t pe_plat_gpuaddr_pixel_chunk(uint32_t object);
uint32_t pe_plat_gpuaddr_pixel_data(uint32_t object);

#define func_80077A64 pe_plat_gpuaddr_texture_page
#define func_80077AA4 pe_plat_gpuaddr_clut
#define func_80071944 pe_plat_gpuaddr_optional_chunk
#define func_80071964 pe_plat_gpuaddr_pixel_chunk
#define func_80071994 pe_plat_gpuaddr_pixel_data
/* Canonical guest-name prototypes for the bindings above. */
int func_80077A64(int tp, int abr, int x, int y);
int func_80077AA4(int x, int y);
uint32_t func_80071944(uint32_t object);
uint32_t func_80071964(uint32_t object);
uint32_t func_80071994(uint32_t object);
#endif
