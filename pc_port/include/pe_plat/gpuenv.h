#ifndef PE_PLAT_GPUENV_H
#define PE_PLAT_GPUENV_H
#include <stdint.h>

/* Draw-environment / draw-mode / draw-offset words (pe_plat2_gpuenv.c,
 * clean-room from the black-box vector sets state60d:
 * docs/evidence/native-state60-20261126, and psx-spx). */

/* Fill a display-environment record: display rectangle x,y,w,h, zeroed
 * screen rectangle and flag bytes; returns env. */
uint32_t pe_plat_gpuenv_display_env(uint32_t env, int x, int y, int width, int height);
/* GP0(E1h) draw-mode word. */
uint32_t pe_plat_gpuenv_draw_mode(int draw_to_display, int dither, uint32_t page);
/* GP0(E5h) drawing-offset word (also the return value of the packet form). */
int pe_plat_gpuenv_draw_offset(int x, int y);
/* Write the drawing-offset word for the signed halfword pair at `coords`
 * into the packet's second word (packet+4) with size byte 2 and a zero tail. */
void pe_plat_gpuenv_draw_offset_packet(uint32_t packet, uint32_t coords);

#define func_800749D8 pe_plat_gpuenv_display_env
#define func_80076150 pe_plat_gpuenv_draw_mode
#define func_800762A0 pe_plat_gpuenv_draw_offset
#define func_80075C04 pe_plat_gpuenv_draw_offset_packet
/* Canonical guest-name prototypes for the bindings above. */
uint32_t func_800749D8(uint32_t env, int x, int y, int width, int height);
uint32_t func_80076150(int draw_to_display, int dither, uint32_t page);
int func_800762A0(int x, int y);
void func_80075C04(uint32_t packet, uint32_t coords);
#endif
