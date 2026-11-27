/* Image-transfer wrappers and workers (guest 0x8007506C LoadImage wrapper, 0x800750CC StoreImage wrapper,
 * 0x8007512C MoveImage, 0x80074E28 rectangle report, 0x80076664 LoadImage worker, 0x800768A0 StoreImage worker),
 * written clean-room from the black-box vector sets gpu2b (docs/evidence/native-gpu2-20261126), the port's host
 * GPU/DMA model API and psx-spx.  Providers: platform/pe_plat2_gpuimg.c (wrappers), pe_plat2_gpuimg_rect.c
 * (rectangle report), pe_plat2_gpuimg_work.c (workers). */
#ifndef PE_PLAT_GPUIMG_H
#define PE_PLAT_GPUIMG_H
#include <stdint.h>
#include "psx_compat.h"   /* RECT, pe_addr_t (psx_compat.h includes this header after its RECT typedef) */

/* Rectangle report (guest 0x80074E28): name is the guest address of the caller's label string. */
void pe_plat_gpuimg_report_rect(pe_addr_t name, const RECT *rect);
#define func_80074E28 pe_plat_gpuimg_report_rect
void func_80074E28(pe_addr_t name, const RECT *rect);

/* Wrappers: the rectangle is a host pointer; one inside guest RAM or the scratchpad is passed on by its guest
 * address, any other (a host stack object) is passed as its two words. */
int pe_plat_gpuimg_load_image(const RECT *rect, pe_addr_t data);
#define func_8007506C pe_plat_gpuimg_load_image
int func_8007506C(const RECT *rect, pe_addr_t data);
int pe_plat_gpuimg_store_image(const RECT *rect, pe_addr_t data);
#define func_800750CC pe_plat_gpuimg_store_image
int func_800750CC(const RECT *rect, pe_addr_t data);
int pe_plat_gpuimg_move_image(const RECT *rect, int destination_x, int destination_y);
#define func_8007512C pe_plat_gpuimg_move_image
int func_8007512C(const RECT *rect, int destination_x, int destination_y);

/* Workers (guest addresses of the rectangle and the data buffer). */
int pe_plat_gpuimg_upload_worker(pe_addr_t rect, pe_addr_t source);
#define func_80076664 pe_plat_gpuimg_upload_worker
int func_80076664(pe_addr_t rect, pe_addr_t source);
int pe_plat_gpuimg_readback_worker(pe_addr_t rect, pe_addr_t destination);
#define func_800768A0 pe_plat_gpuimg_readback_worker
int func_800768A0(pe_addr_t rect, pe_addr_t destination);
/* Callers that hold the rectangle as two words instead of guest memory (position word, size word). */
int pe_plat_gpuimg_upload_inline(uint32_t position, uint32_t size, pe_addr_t source);
#define PE_func_80076664_Inline8 pe_plat_gpuimg_upload_inline
int PE_func_80076664_Inline8(uint32_t position, uint32_t size, pe_addr_t source);
int pe_plat_gpuimg_readback_inline(uint32_t position, uint32_t size, pe_addr_t destination);
#define PE_StoreImageInline8 pe_plat_gpuimg_readback_inline
int PE_StoreImageInline8(uint32_t position, uint32_t size, pe_addr_t destination);

/* Names the replaced translations exported; kept as aliases for the callers outside this family that still use
 * them by their pe_plat name (pe_libgpu.c, pe_plat_gpu_tim.c, pe_plat_gpu_swap_host.c, tests).  To be retired with
 * those callers. */
#define pe_plat_gpu_report_rect_host    pe_plat_gpuimg_report_rect
#define pe_plat_gpu_load_rect_host      pe_plat_gpuimg_load_image
#define pe_plat_gpu_store_rect_host     pe_plat_gpuimg_store_image
#define pe_plat_gpu_move_image_host     pe_plat_gpuimg_move_image
#define pe_plat_gpu_upload_worker_host  pe_plat_gpuimg_upload_worker
#define pe_plat_gpu_readback_worker_host pe_plat_gpuimg_readback_worker
#define pe_plat_gpu_upload_inline_host  pe_plat_gpuimg_upload_inline
#define pe_plat_gpu_readback_inline_host pe_plat_gpuimg_readback_inline
#endif
