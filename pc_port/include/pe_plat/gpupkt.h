/* GPU packet setters (libgpu-style ordering-table packets): clean-room provider
 * pc_port/platform/pe_plat2_gpupkt.c, written from black-box vectors
 * (state60c, docs/evidence/native-state60-20261126) and psx-spx (GPU GP0
 * commands, linked-list packet header).  These bindings replace the rows of
 * the same guest names that gpu_primitive.h used to carry. */
#ifndef PE_PLAT_GPUPKT_H
#define PE_PLAT_GPUPKT_H
#include <stdint.h>

void pe_plat_gpupkt_poly_f3(uint32_t packet);
void pe_plat_gpupkt_poly_g3(uint32_t packet);
void pe_plat_gpupkt_poly_ft4(uint32_t packet);
void pe_plat_gpupkt_poly_g4(uint32_t packet);
void pe_plat_gpupkt_poly_gt4(uint32_t packet);
void pe_plat_gpupkt_sprite(uint32_t packet);
void pe_plat_gpupkt_tile1(uint32_t packet);
void pe_plat_gpupkt_tile(uint32_t packet);
void pe_plat_gpupkt_line_f2(uint32_t packet);
void pe_plat_gpupkt_set_semitrans(uint32_t packet, int enabled);
void pe_plat_gpupkt_set_shade_texture(uint32_t packet, int enabled);
/* draw_to_display is the GP0(E1h) bit 10 flag, dither bit 9 (argument order
 * as the guest passes them: packet, draw-to-display, dither, tpage). */
uint32_t pe_plat_gpupkt_draw_page(uint32_t packet, uint32_t draw_to_display,
                                   uint32_t dither, uint32_t page);
int pe_plat_gpupkt_append_packet(uint32_t head, uint32_t tail);

#define func_80077B64 pe_plat_gpupkt_poly_f3
#define func_80077B84 pe_plat_gpupkt_poly_g3
#define func_80077BA4 pe_plat_gpupkt_poly_ft4
#define func_80077BC4 pe_plat_gpupkt_poly_g4
#define func_80077BE4 pe_plat_gpupkt_poly_gt4
#define func_80077C04 pe_plat_gpupkt_sprite
#define func_80077C24 pe_plat_gpupkt_tile1
#define func_80077C44 pe_plat_gpupkt_tile
#define func_80077C64 pe_plat_gpupkt_line_f2
#define func_80077B04 pe_plat_gpupkt_set_semitrans
#define func_80077B34 pe_plat_gpupkt_set_shade_texture
#define func_80077C84 pe_plat_gpupkt_draw_page
#define func_80077CB4 pe_plat_gpupkt_append_packet

/* Canonical guest-name prototypes for the bindings above. */
void func_80077B64(uint32_t packet);
void func_80077B84(uint32_t packet);
void func_80077BA4(uint32_t packet);
void func_80077BC4(uint32_t packet);
void func_80077BE4(uint32_t packet);
void func_80077C04(uint32_t packet);
void func_80077C24(uint32_t packet);
void func_80077C44(uint32_t packet);
void func_80077C64(uint32_t packet);
void func_80077B04(uint32_t packet, int enabled);
void func_80077B34(uint32_t packet, int enabled);
uint32_t func_80077C84(uint32_t packet, uint32_t draw_to_display, uint32_t dither, uint32_t page);
int func_80077CB4(uint32_t head, uint32_t tail);

#endif
