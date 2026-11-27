/* libgpu primitive / ordering-table entry points, the OT clear and its DMA6
 * worker, the record-pool initialiser and the stream range setter: clean-room
 * providers pc_port/platform/pe_plat2_gpuprim*.c, written from the black-box
 * vector set gpu2c (docs/evidence/native-gpu2-20261126), the port's host
 * GPU/DMA model API and psx-spx.  These bindings replace the rows of the same
 * guest names that psx_compat.h and cd_stream.h used to carry. */
#ifndef PE_PLAT_GPUPRIM_H
#define PE_PLAT_GPUPRIM_H
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_plat/rect.h"

/* DrawPrim: slot 15 (0), then slot 5 (packet + 4, byte count at packet + 3). */
int pe_plat_gpuprim_draw_prim(uint32_t packet);
/* DrawOTag: slot 3 (slot 6 argument, ot, 0, 0). */
int pe_plat_gpuprim_draw_otag(uint32_t ot);
/* ClearOTagR: slot 11 clears the table, then the terminator node is written. */
void pe_plat_gpuprim_clear_otag_reverse(uint32_t ot, int count);
/* Slot-11 worker: DMA channel 6 (OTC) fill, polled to completion; returns count. */
int pe_plat_gpuprim_otc_worker(uint32_t ot, int count);
/* ClearImage: fill a rectangle with an RGB colour through slot 2 / slot 3. */
int pe_plat_gpuprim_clear_image(RECT *rect, uint8_t red, uint8_t green, uint8_t blue);
/* PutDrawEnv / DrawOTagEnv: build and issue the environment packet, copy the
 * environment into the library's current-environment block. */
uint32_t pe_plat_gpuprim_put_draw_env(uint32_t env);
void pe_plat_gpuprim_draw_otag_env(uint32_t ot, uint32_t env);
/* Environment packet builder (SetDrawEnv role): fills the packet at `packet` from the environment record `env`
 * through the five packet-word functions; returns the packet's word count. */
int pe_plat_gpuprim_set_draw_env(uint32_t packet, uint32_t env);
/* Record-pool initialiser (stores base and count, then runs the worker) and
 * its worker (resets the pool state words and the ring status); stream range. */
void pe_plat_gpuprim_pool_init(uint32_t record_base, uint32_t record_count);
void pe_plat_gpuprim_pool_clear(uint32_t record_base, uint32_t record_count);
void pe_plat_gpuprim_stream_set_range(uint32_t mode, uint32_t start, uint32_t end);

#define func_80075358 pe_plat_gpuprim_draw_prim
#define func_800753B4 pe_plat_gpuprim_draw_otag
#define func_800752AC pe_plat_gpuprim_clear_otag_reverse
#define func_80076354 pe_plat_gpuprim_otc_worker
#define func_80074F44 pe_plat_gpuprim_clear_image
#define func_80075424 pe_plat_gpuprim_put_draw_env
#define func_800754E4 pe_plat_gpuprim_draw_otag_env
#define func_80075EE0 pe_plat_gpuprim_set_draw_env
#define func_8007A214 pe_plat_gpuprim_pool_init
#define func_8007A244 pe_plat_gpuprim_pool_clear
#define func_8007C544 pe_plat_gpuprim_stream_set_range
/* Canonical guest-name prototypes for the bindings above. */
int func_80075358(uint32_t packet);
int func_800753B4(uint32_t ot);
void func_800752AC(uint32_t ot, int count);
int func_80076354(uint32_t ot, int count);
int func_80074F44(RECT *rect, uint8_t red, uint8_t green, uint8_t blue);
uint32_t func_80075424(uint32_t env);
void func_800754E4(uint32_t ot, uint32_t env);
int func_80075EE0(uint32_t packet, uint32_t env);
void func_8007A214(uint32_t record_base, uint32_t record_count);
void func_8007A244(uint32_t record_base, uint32_t record_count);
void func_8007C544(uint32_t mode, uint32_t start, uint32_t end);

/* Host-only (not a guest function): DrawPrim of `count` host-held command words. */
int PE_func_80075358_Transient(const uint32_t *words, uint8_t count);
#endif
