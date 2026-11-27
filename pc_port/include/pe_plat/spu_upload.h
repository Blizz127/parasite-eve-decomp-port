/* SPU data upload family: the upload (0x800850F4), its prepare step (0x800850C0), the DMA-complete wait
 * (0x80085174) and the DMA-complete callback (0x80085098), clean-room host implementations in
 * platform/pe_plat2_spu_upload.c.  Written clean-room from the black-box vector sets cdgen48
 * (docs/evidence/native-cdx-20261126/cdgen48), psx-spx (SPU / DMA channel 4) and the public headers;
 * no SDK source, decompiled source, tables or disassembly were read.
 *
 * Behaviour (what the vectors show):
 *   upload(src, size)   prepare, then the SPU RAM write adapter pe_plat_cdxspu_spu_write(src, size) with the
 *                       caller's size unclamped (the adapter owns the 0x7EFF0 clamp).
 *   prepare()           guest word 0x8009D24C := 1 (transfer busy), then the transfer callback word is set to
 *                       the guest address of the callback (0x80085098) through pe_plat_spub_set_callback.
 *   wait()              returns at once unless 0x8009D24C reads 1; polls the word while it reads 1.
 *   callback()          the transfer callback word is cleared (pe_plat_spub_set_callback(0)), then
 *                       0x8009D24C := 0.  Reached only through the DMA interrupt delivery of the host model.
 *
 * Host contracts (not visible in the vectors; documented in the provider): the upload first lets a still
 * pending host DMA complete (PE_SpuDma_DeliverPending) and issues nothing when that ended in a stop; the wait
 * gives the DMA interrupt opportunity once per poll (PE_SpuDma_Service) and follows the hardware-wait
 * contract (ShouldStop exit, one pe_plat_sio_poll_tick per poll, stuck-wait budget then
 * PE_Port_RequestStop with a diagnostic).
 * The entry names use the prefix pe_plat_spuupload_. */
#ifndef PE_PLAT_SPU_UPLOAD_H
#define PE_PLAT_SPU_UPLOAD_H
#include <stdint.h>

void pe_plat_spuupload_upload(uint32_t src, uint32_t size);
void pe_plat_spuupload_prepare(void);
void pe_plat_spuupload_wait(void);
void pe_plat_spuupload_callback(void);

#define func_800850F4 pe_plat_spuupload_upload
#define func_800850C0 pe_plat_spuupload_prepare
#define func_80085174 pe_plat_spuupload_wait
#define func_80085098 pe_plat_spuupload_callback
/* Canonical guest-name prototypes for the bindings above. */
void func_800850F4(uint32_t src, uint32_t size);
void func_800850C0(void);
void func_80085174(void);
void func_80085098(void);
#endif
