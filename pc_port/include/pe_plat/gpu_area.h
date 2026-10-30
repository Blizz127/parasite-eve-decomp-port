/* Texture-window and drawing-area packets (psx-spx GP0(E2h)/(E3h)/(E4h));
 * host model in platform/pe_plat_gpu_area.c. */
#ifndef PE_PLAT_GPU_AREA_H
#define PE_PLAT_GPU_AREA_H
#include <stdint.h>
void pe_plat_gpu_set_tex_window(uint32_t packet, uint32_t rect);
void pe_plat_gpu_set_draw_area(uint32_t packet, uint32_t rect);
void pe_plat_gpu_tex_window_values(uint32_t packet, const int16_t rect[4]);
void pe_plat_gpu_draw_area_values(uint32_t packet, const int16_t rect[4]);
#define func_80075B4C pe_plat_gpu_set_tex_window
#define func_80075B84 pe_plat_gpu_set_draw_area
#endif
