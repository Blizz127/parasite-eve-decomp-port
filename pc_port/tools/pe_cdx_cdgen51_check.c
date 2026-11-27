/* Checker for the cdgen51 set (pe_cdx_cdgen51_vectors.py): libspu SpuClearReverbWorkArea, retail 0x8008D610.  Scaffold pe_cdx_common_canary.h (the int3
 * pe_cdx_common.h: canary seeding, store-footprint category, controls noop / flip / retflip / storeflip / poke0 / same).  One provider object per build,
 * linked as a black box: pc_port/platform/pe_stream.c of the int3 tree (read-only), entry PE_SpuReverb_ClearWorkArea (the host backend the port binds the
 * reverb clear to); the object also defines the stream bring-up / dispatch / mode-transition functions (dead duplicates of the clean-room SPU family) and
 * references host symbols they need; those are trap stand-ins here (a call fails the case).  Seams the entry reaches (the checker's stand-ins):
 *   pe_plat_spub_heap_contains      = retail 0x80085BB4 (arity 1, scripted result),
 *   PE_Event_ConsumeSpuDma(handle)  = the event wait, retail 0x8008D7B0 (arity 1 = the handle, scripted per-call result),
 *   PE_SpuRegister_LoadU16 / StoreU16(offset)  = the SPU register 0x1F801C00 + offset (replayed read / ordered register store),
 *   PE_SpuDma_DeliverPending / PE_SpuDma_Service  = host-only (counted, return 1),
 *   PE_SpuDma_Begin(source, destination, size, callback)  = the SPU transfer issue.  Strict (default): host-only (counted, returns 1), so the DMA4 registers,
 *       SPU_DELAY and the bookkeeping words the original writes are the provider's own business;  -DC51_MAP_DMA4=1 (mapped): the stand-in performs the
 *       register-level expansion the vectors show for the issue (SPU_DELAY read and rewrite, bookkeeping words 0x8009B450 / 0x8009B454, DMA channel 4 MADR /
 *       BCR / CHCR stores) and the register stand-in consumes the original's read-back of its 0x1A6 / 0x1AA writes: the most generous seam mapping, so only the provider's own logic is measured.
 * Build lines (executed verbatim by pe_cdx_cdgen51_run.sh; $O = scratch output directory):
 * BUILD-strict: cc -std=gnu11 -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen51_check.c $O/pe_stream.o -o $O/c51_strict
 * BUILD-mapped: cc -std=gnu11 -Wall -Wextra -Werror -DC51_MAP_DMA4=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_cdgen51_check.c $O/pe_stream.o -o $O/c51_mapped
 * Run: $O/c51_<name> clear.txt [noop|flip|retflip|storeflip|poke0|same]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_DEBUG=1, CDX_TRACE=1) */
#include "pe_cdx_common_canary.h"

int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
CALLEE(pe_plat_spub_heap_contains, 0x80085BB4u, 1)
int PE_Event_ConsumeSpuDma(int handle) { return (int)cdx_callee(0x8008D7B0u, 1, (uint32_t)handle, 0, 0, 0, 0); }
uint16_t PE_SpuRegister_LoadU16(uint32_t offset) { uint32_t v; if (mmio_read(0x1F801C00u + offset, 2, &v)) return (uint16_t)v; return 0; }
void PE_SpuRegister_StoreU16(uint32_t offset, uint16_t value)
{
    PE_StoreU16(0x1F801C00u + offset, value);
#if C51_MAP_DMA4
    /* mapped: the original reads back its transfer-address (0x1A6) and control (0x1AA) register writes; the stand-in consumes those recorded reads */
    if (offset == 0x1A6u || offset == 0x1AAu) { uint32_t d; (void)mmio_read(0x1F801C00u + offset, 2, &d); }
#endif
}
int PE_SpuDma_DeliverPending(void) { host_only_calls++; return 1; }
int PE_SpuDma_Service(void) { host_only_calls++; return 1; }
int PE_SpuDma_Begin(pe_addr_t source, uint32_t destination, uint32_t size, pe_addr_t callback)
{
    (void)destination; (void)callback;
#if C51_MAP_DMA4
    {
        uint32_t d = 0, blocks = (size + 63u) >> 6;
        (void)mmio_read(0x1F801014u, 4, &d);
        PE_StoreU32(0x1F801014u, (d & 0xF0FFFFFFu) | 0x20000000u);
        PE_StoreU32(0x8009B450u, source);
        PE_StoreU32(0x8009B454u, blocks);
        PE_StoreU32(0x1F8010C0u, source);
        PE_StoreU32(0x1F8010C4u, (blocks << 16) | 0x10u);
        PE_StoreU32(0x1F8010C8u, 0x01000201u);
    }
#else
    (void)source; (void)size;
    host_only_calls++;
#endif
    return 1;
}
/* referenced by the other functions of the same object: a call fails the case */
TRAP(func_80085084) TRAP(PE_Event_Enable) TRAP(PE_Event_Open) TRAP(PE_Event_SpuDmaEnabled) TRAP(pe_plat_cdxspu_install_dma_event)
TRAP(pe_plat_ctl_counter_setup) TRAP(pe_plat_ctl_counter_start) TRAP(pe_plat_spua_set_reverb_enabled) TRAP(pe_plat_spub_set_common_attributes)
TRAP(pe_plat_spub_voice_volume) TRAP(pe_plat_spu_set_reverb_mode_type) TRAP(pe_plat_spuupload_upload) TRAP(PE_SpuDma_WaitForCompletion)
int PE_SpuReverb_ClearWorkArea(int mode);
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    if (entry != 0x8008D610u) return -1;
    if (mutant == 1) { *ret = 0; return 1; }
    *ret = (uint32_t)PE_SpuReverb_ClearWorkArea((int)a[0]);
    return 1;
}
