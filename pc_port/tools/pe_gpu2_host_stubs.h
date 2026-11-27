/* Host plumbing every sw-vec-gpu2 checker build links (clean-room and smoke alike): guest-call entry points, stop and
 * RAM-range hooks, and stubs for the guest functions that are callees of the code under test (each guarded by
 * G2_REAL_<name> when the batch's provider is the code under test).  Prototypes are those declared in
 * pc_port/platform/pe_guestcode.h, pe_guest_ram.h and pc_port/bootstrap/{game_port,pe_bootstrap}.h (prototype lines
 * only). */
#ifndef PE_GPU2_HOST_STUBS_H
#define PE_GPU2_HOST_STUBS_H
#include "game_port.h"

int PE_GuestCall(const char *site, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3)
{ (void)site; (void)arity; return (int)g2_call(fn, (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3); }
int PE_GuestCallN(const char *site, pe_addr_t fn, unsigned arity, const uintptr_t *args)
{ (void)site; (void)arity; return (int)g2_call(fn, (uint32_t)args[0], (uint32_t)args[1], (uint32_t)args[2], (uint32_t)args[3]); }
int PE_GuestHook_Call(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1,
                      uintptr_t a2, uintptr_t a3, int *ret)
{
    int r = (int)g2_call(fn, (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3);
    (void)slot; (void)caller; (void)arity;
    if (ret) *ret = r;
    return !g2_fail;
}
void PE_GuestHook_Refuse(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1,
                         uintptr_t a2, uintptr_t a3, const char *why)
{ (void)slot; (void)caller; (void)fn; (void)arity; (void)a0; (void)a1; (void)a2; (void)a3; (void)why; g2_fail = 1; }
int Bootstrap_ReturnInt(const char *symbol, const char *caller, int value) { (void)symbol; (void)caller; return value; }
int Bootstrap_ReturnInt4Indirect(const char *symbol, const char *caller, int value, uintptr_t target, uintptr_t arg0,
                                 uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, const void *payload, uint32_t payload_size)
{ (void)symbol; (void)caller; (void)target; (void)arg0; (void)arg1; (void)arg2; (void)arg3; (void)payload; (void)payload_size; return value; }
void Bootstrap_ReturnVoid(const char *symbol, const char *caller) { (void)symbol; (void)caller; }
void Bootstrap_RecordArg4Indirect(const char *symbol, const char *caller, uintptr_t target, uintptr_t arg0, uintptr_t arg1,
                                  uintptr_t arg2, uintptr_t arg3, const void *payload, uint32_t payload_size)
{ (void)symbol; (void)caller; (void)target; (void)arg0; (void)arg1; (void)arg2; (void)arg3; (void)payload; (void)payload_size; }
void PE_Port_RequestStop(PEPortStopReason reason) { (void)reason; if (!g2_fail) snprintf(g2_msg, sizeof g2_msg, "provider requested a stop (PE_Port_RequestStop)"); g2_fail = 1; }
int PE_Port_ShouldStop(void) { return (int)g2_fail; }
unsigned PE_Port_StopEpoch(void) { return 0; }
int PE_Port_EscapeArmed(void) { return 0; }
PEPortDmaIrqCheckpointResult PE_Port_ServiceDmaIrqCheckpoint(void) { PEPortDmaIrqCheckpointResult r; memset(&r, 0, sizeof r); return r; }
void PE_Decomp_NoteStopDetail(const char *text) { (void)text; }
bool PE_RangeIsRam(pe_addr_t a, size_t n) { (void)a; (void)n; return true; }
int PE_MMIO_IsModelled(pe_addr_t a, size_t n) { (void)a; (void)n; return 1; }
void *PE_Translate(pe_addr_t a, size_t n) { (void)n; return g2_ram + (a & (G2_MEM - 1)); }
pe_addr_t PE_HostToGuest(const volatile void *host)
{ return 0x80000000u | (uint32_t)((const volatile uint8_t *)host - (const volatile uint8_t *)g2_ram); }

/* guest-name hooks of other libraries the providers call (pe_plat headers) */
#ifndef G2_REAL_DMA_CALLBACK
uint32_t pe_plat_dma_callback(uint32_t channel, uint32_t handler) { return g2_call(0x80073CF4u, channel, handler, 0, 0); }
#endif
#ifndef G2_REAL_IRQ_SET_MASK
uint16_t pe_plat_irq_set_mask(uint16_t m) { return (uint16_t)g2_call(0x80073E10u, m, 0, 0, 0); }
#endif
#ifndef G2_REAL_IRQ_SET_MASK
uint16_t pe_plat_cdxetc_set_intr_mask(uint16_t m) { return (uint16_t)g2_call(0x80073E10u, m, 0, 0, 0); }   /* func_80073E10 binding since pe_plat/cdx_etc.h */
#endif
#ifndef G2_REAL_CLEAR_WORKER
int pe_plat_gpu_clear_worker_host(pe_addr_t rect, uint32_t color) { return (int)g2_call(0x80076434u, rect, color, 0, 0); }
#endif
#ifndef G2_REAL_READBACK_WORKER
int pe_plat_gpuimg_readback_worker(pe_addr_t r, pe_addr_t d) { return (int)g2_call(0x800768A0u, r, d, 0, 0); }
#endif
#ifndef G2_REAL_UPLOAD_WORKER
int pe_plat_gpuimg_upload_worker(pe_addr_t r, pe_addr_t s) { return (int)g2_call(0x80076664u, r, s, 0, 0); }
#endif
#ifndef G2_REAL_DMA_START
int pe_plat_gpu_dma_start(uint32_t p, uint32_t a) { return (int)g2_call(0x80076B98u, p, a, 0, 0); }
#endif
#ifndef G2_REAL_SWAP
int32_t pe_plat_gpu_swap(pe_addr_t w, pe_addr_t a, int32_t n, uint32_t x) { return (int32_t)g2_call(0x80076C34u, w, a, (uint32_t)n, x); }
#endif
#ifndef G2_REAL_SWAP_INLINE8
int32_t pe_plat_gpu_swap_inline8(pe_addr_t w, uint32_t w0, uint32_t w1, uint32_t x) { (void)w0; (void)w1; return (int32_t)g2_call(0x80076C34u, w, 0, 8, x); }
#endif
#ifndef G2_REAL_DRAWSYNC
int pe_plat_gpu_drawsync(int32_t mode) { return (int)g2_call(0x80074DC0u, (uint32_t)mode, 0, 0, 0); }
#endif
#ifndef G2_REAL_UPLOAD_INLINE
int pe_plat_gpuimg_upload_inline(uint32_t w0, uint32_t w1, pe_addr_t src) { (void)w0; (void)w1; (void)src; return 0; }
#endif
#ifndef G2_REAL_READBACK_INLINE
int pe_plat_gpuimg_readback_inline(uint32_t pos, uint32_t size, pe_addr_t dst) { (void)pos; (void)size; (void)dst; return 0; }
#endif
#ifndef G2_REAL_CLEAR_INLINE
int pe_plat_gpu_clear_inline_host(uint32_t words[2], uint32_t color) { (void)words; (void)color; return 0; }
#endif
#ifndef G2_REAL_SEED
uint32_t pe_plat_gpu_seed_deadline_host(void) { return g2_call(0x800773D0u, 0, 0, 0, 0); }
#endif
#ifndef G2_REAL_SEED
uint32_t pe_plat_gpuq_seed_deadline(void) { return g2_call(0x800773D0u, 0, 0, 0, 0); }   /* func_800773D0 binding since gpu2a */
#endif
#ifndef G2_REAL_VSYNC_QUERY
uint32_t pe_plat_gpu_host_vsync_query(void) { return g2_call(0x80073A44u, 0xFFFFFFFFu, 0, 0, 0); }   /* host hook (pe_plat/gpu_host.h): the guest VSync(-1) the clean-room seed reads */
#endif
#ifndef G2_REAL_TIMEOUT
int32_t pe_plat_gpu_timeout(void) { return (int32_t)g2_call(0x80077404u, 0, 0, 0, 0); }
#endif
uint32_t HostFB_VSync(int mode) { return g2_call(0x80073A44u, (uint32_t)mode, 0, 0, 0); }   /* func_80073A44 = HostFB_VSync (psx_compat.h) */
#ifndef G2_REAL_REPORT_RECT
void pe_plat_gpuimg_report_rect(uint32_t name, const void *rect) { (void)g2_call(0x80074E28u, name, PE_HostToGuest(rect), 0, 0); }
void pe_plat_gpu_report_rect_host(uint32_t name, const void *rect) { (void)g2_call(0x80074E28u, name, PE_HostToGuest(rect), 0, 0); }   /* pre-gpuimg name: pe_plat2_gpuprim.c still calls it */
#endif
int PE_DispatchClearRect(uint32_t worker, void *rect, uint32_t color) { (void)rect; return (int)g2_call(0x80076C34u, worker, 0, 8, color); }   /* host seam: the checker only passes guest RECTs, never reached */
uint32_t g2_stub_80075EE0(uint32_t a, uint32_t b) { return g2_call(0x80075EE0u, a, b, 0, 0); }   /* callee stub under a neutral name; a provider that references the guest name func_80075EE0 is linked with -Wl,--defsym=func_80075EE0=g2_stub_80075EE0 */
uint32_t pe_plat_bios_memcpy(uint32_t dst, uint32_t src, int32_t n) { return g2_call(0x80071A34u, dst, src, (uint32_t)n, 0); }
int32_t pe_plat_gpu_dma_queue_run(int32_t mode) { return (int32_t)g2_call(0x80077294u, (uint32_t)mode, 0, 0, 0); }
#ifndef G2_REAL_POOL_WORKER
void pe_plat_gpuprim_pool_clear(uint32_t a, uint32_t b) { (void)g2_call(0x8007A244u, a, b, 0, 0); }   /* func_8007A244 (pe_plat/gpuprim.h) */
#endif
void pe_plat_cd_clear_ring_status(int start, unsigned int count) { (void)g2_call(0x8007C444u, (uint32_t)start, count, 0, 0); }
#ifndef G2_REAL_OT_WORKER
int pe_plat_gpuprim_otc_worker(pe_addr_t ot, int n) { return (int)g2_call(0x80076354u, ot, (uint32_t)n, 0, 0); }   /* func_80076354 (pe_plat/gpuprim.h) */
#endif
#ifndef G2_REAL_WORDS
int pe_plat_ctl_gpu_transfer_words(uint32_t source, int count) { return (int)g2_call(0x80076B58u, source, (uint32_t)count, 0, 0); }   /* func_80076B58 (pe_plat/ctl.h) */
#endif
#endif
