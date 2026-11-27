/* Bind the SPU installer / init chain, the two stream-feed leaves and the two SPU DMA adapters of
 * batch cdgen44 (docs/evidence/native-cdx-20261126, family cdx-spu) to the clean-room host
 * implementations in platform/pe_plat2_cdx_spu.c.  Written clean-room from the black-box vector sets
 * cdgen44 (docs/evidence/native-cdx-20261126), the recorded seam contracts and psx-spx; no SDK source,
 * decompiled source, tables or disassembly were read.
 *
 * Seam contract (a host seam the checkers stand in for; part of this binding):
 *   int PE_SpuDma_InstallIrq(pe_addr_t handler)  (pe_plat/spu_dma_hw.h)
 *     returns NONZERO when the host SPU DMA model accepted the completion handler (1 = installed) and 0
 *     when it did not.  pe_plat_cdxspu_install_dma_event registers the DMA channel 4 callback and the
 *     BIOS event (OpenEvent / EnableEvent, class 0xF0000009) ONLY when the seam reports nonzero; on
 *     zero it still takes the one-shot guard and the critical section but registers nothing.  A
 *     stand-in that returns 0 therefore removes those calls from the recorded sequence; the vector
 *     lane's stand-in must return nonzero (cdgen44 expects the registrations).
 *
 * Host-model addition used by the two DMA adapters (pe_plat_cdxspu_spu_write / _spu_read): they drive the
 * SPU registers, SPU_DELAY (0x1F801014) and the DMA channel 4 registers (0x1F8010C0/C4/C8) like the
 * original does; the MMIO layer (pe_mmio.c) now models those registers and starts the transfer on a CHCR
 * write with the start bit (PE_SpuDma_Dma4Store, pe_plat/spu_dma_hw.h). */
#ifndef PE_PLAT_CDX_SPU_H
#define PE_PLAT_CDX_SPU_H
#include <stdint.h>

void pe_plat_cdxspu_install_dma_event(void);
void pe_plat_cdxspu_init_default(void);
void pe_plat_cdxspu_init_wrapper(int mode);
void pe_plat_cdxspu_reverb_transition(uint32_t mode);
int pe_plat_cdxspu_stream_dispatch(void);
void pe_plat_cdxspu_stream_bringup(void);
uint32_t pe_plat_cdxspu_spu_write(uint32_t source, uint32_t size);
uint32_t pe_plat_cdxspu_spu_read(int guest_destination, unsigned int size);

#define func_8007D15C pe_plat_cdxspu_install_dma_event
#define func_8007D054 pe_plat_cdxspu_init_default
#define func_8007D074 pe_plat_cdxspu_init_wrapper
#ifndef PE_CDXSPU_KEEP_LEGACY_STREAM
#define func_8008CB54 pe_plat_cdxspu_reverb_transition
#define func_8008CBA8 pe_plat_cdxspu_stream_dispatch
#define func_80085644 pe_plat_cdxspu_stream_bringup
#endif
#define func_80085E54 pe_plat_cdxspu_spu_write
#define func_80085DF4 pe_plat_cdxspu_spu_read
/* Canonical guest-name prototypes for the bindings above (generated
 * from the pe_plat prototypes; tools read callee signatures from these). */
void func_8007D15C(void);
void func_8007D054(void);
void func_8007D074(int mode);
void func_8008CB54(uint32_t mode);
int func_8008CBA8(void);
void func_80085644(void);
uint32_t func_80085E54(uint32_t source, uint32_t size);
uint32_t func_80085DF4(int guest_destination, unsigned int size);
#endif
