/* Vector check for batch cdgen48 (pe_cdx_vectors.py cdgen48): the SPU data upload func_800850F4(src, size) and its
 * DMA-complete wait func_80085174() the DMA-complete callback func_80085098() and the prepare step func_800850C0().  Black-box: the provider is linked as a built object (never read here).
 *
 * What a case compares (scaffold pe_cdx_common.h):
 *   - the register (0x1F80xxxx) STORES in the original's order, width and value (TSA 0x1F801DA6, SPUCNT 0x1F801DAA,
 *     data port 0x1F801DA8, SPU_DELAY 0x1F801014, DMA4 MADR/BCR/CHCR 0x1F8010C0/C4/C8);
 *   - the register READS, replayed in the original's order and width (SPUSTAT 0x1F801DAE, SPUCNT, TSA readback, SPU_DELAY):
 *     reading more, fewer or other registers fails (category register-read-replay);
 *   - the DMA-completion flag 0x8009D24C: for func_80085174 its loads are replayed in the same ordered queue as the register
 *     reads (replay_extra).  The clearing of the flag is a HARNESS MODEL (the original's DMA4 interrupt handler is not run
 *     by the interpreter): the vectors say "the word reads 1 for m reads, then a terminal value T"; a provider must poll
 *     through PE_LoadU32 exactly m + 1 times (a host wait service that never reads the word fails here by design);
 *   - the final RAM image (full diff), the callee records of the 'bound' vector variant (80085F44, 80085E54) with their
 *     arguments and the number of distinct bytes written before each call.
 * Variants (vector file name): vectors.txt / noop (register level, nothing stopped), bound.txt (80085F44 and 80085E54 are
 * stand-ins), fault.txt (register level + stuck TSA / sticky SPUCNT models: 3841-read timeouts).
 *
 * Stand-ins (all weak, so a provider that defines the symbol itself wins): the two bound host callees of the 'bound'
 * variant (80085F44 under pe_plat_spub_set_callback / func_80085F44, 80085E54 under pe_plat_spu_write / pe_plat_cdxspu_spu_write /
 * func_80085E54) record a callee; the
 * seams of the legacy translated file (PE_SpuDma_*, pe_plat_spu_write_io, pe_plat_spu_dma_priority_enabled) are host-only
 * recorders, counted and never compared (they have no retail counterpart: retail programs DMA4 through registers);
 * PE_Port_RequestStop fails the case; PE_Port_ShouldStop returns 0; PE_RangeIsRam is true for physical RAM ranges.
 * LINKING: bound sets (*_bound.txt) = the provider object alone (linking the real 80085E54 / 80085F44 providers overrides the weak recorders
 * and their MMIO stores appear unexpected); register-level sets = provider + the clean-room 80085E54 (cdx_spu) + 80085F44 (spu_b) objects.
 * A provider with other undefined symbols needs its own stand-ins in a second .c file on the command line.
 * CDX48_DEBUG_HOST=1 prints the host-only calls of the first failing cases (arguments).
 *
 * BUILD: see pc_port/tools/pe_cdx_cdgen48_run.sh check (bound sets: provider pe_plat2_spu_upload.c.o alone; register-level sets: -DCDX48_REGSTUBS plus the pe_plat2_cdx_spu / spu_a / spu_b objects)
 * Run: ./check48 vectors.txt [noop|flip]     (CDX_CATEGORIES=1 prints failure categories)
 */
#include "pe_cdx_common.h"
/* The clean-room provider (platform/pe_plat2_spu_upload.c) exports pe_plat_spuupload_*; the header maps the retail names the
 * vectors use onto them.  -DCDX48_LEGACY_NAMES checks an object that defines the raw func_ names instead. */
#ifndef CDX48_LEGACY_NAMES
#include "pe_plat/spu_upload.h"
#endif

#define WEAK __attribute__((weak))
__attribute__((constructor)) static void init48(void) { replay_extra = 0x8009D24Cu; timing_needs_logged_stores = 1; }

/* ---- retail callees the 'bound' variant stops (host names from the public pe_plat headers) ---- */
/* retail 80085F44 (SetTransferCallback, 1 arg): pe_plat_spub_set_callback (pe_plat/spu_b.h) and the raw name under a neutral stub name (guard-clean; mapped with -Dfunc_80085F44=cdx48_stub_80085F44 by the run script) */
WEAK void pe_plat_spub_set_callback(int callback) { cdx_callee(0x80085F44u, 1, (uint32_t)callback, 0, 0, 0, 0); }
WEAK void cdx48_stub_80085F44(int callback) { cdx_callee(0x80085F44u, 1, (uint32_t)callback, 0, 0, 0, 0); }
/* retail 80085E54 (register-level SPU DMA write, 2 args; the CLAMP of size to 0x7EFF0 lives inside it, so it receives the caller's
 * size unclamped): the legacy name pe_plat_spu_write, the clean-room name pe_plat_cdxspu_spu_write (pe_plat/cdx_spu.h on sw-cdx-spu),
 * and the raw name under a neutral stub name (guard-clean; -Dfunc_80085E54=cdx48_stub_80085E54 in the run script) */
WEAK uint32_t pe_plat_spu_write(uint32_t source, uint32_t size) { return cdx_callee(0x80085E54u, 2, source, size, 0, 0, 0); }
WEAK uint32_t pe_plat_cdxspu_spu_write(uint32_t source, uint32_t size) { return cdx_callee(0x80085E54u, 2, source, size, 0, 0, 0); }
WEAK uint32_t cdx48_stub_80085E54(uint32_t source, uint32_t size) { return cdx_callee(0x80085E54u, 2, source, size, 0, 0, 0); }

/* ---- host-only seams of the legacy translated file (counted, not compared) ---- */
static unsigned host_log_n; static char host_log[64][96];
static void host_note(const char *name, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    host_only_calls++;
    if (escape_armed && host_only_calls > 2000000u) longjmp(case_escape, 1);   /* a native looping on a host wait */
    if (getenv("CDX48_DEBUG_HOST") && host_log_n < 64) snprintf(host_log[host_log_n++], sizeof host_log[0], "%s(%X,%X,%X,%X)", name, a, b, c, d);
}
WEAK int PE_SpuDma_Begin(pe_addr_t source, uint32_t destination, uint32_t requested_size, pe_addr_t callback)
{ host_note("PE_SpuDma_Begin", source, destination, requested_size, callback); return 1; }
WEAK int PE_SpuDma_BeginRead(pe_addr_t d, uint32_t s, uint32_t n, pe_addr_t cb) { host_note("PE_SpuDma_BeginRead", d, s, n, cb); return 1; }
WEAK int PE_SpuDma_DeliverPending(void) { host_note("PE_SpuDma_DeliverPending", 0, 0, 0, 0); return 1; }
/* The host interrupt opportunity of the wait: the vectors model the completion by the ordered D24C replay, so the stand-in reports that an
 * interrupt ran (1) on every poll; the provider stops the port on 0 (nothing pending), which fails the case through PE_Port_RequestStop. */
WEAK int PE_SpuDma_Service(void) { host_note("PE_SpuDma_Service", 0, 0, 0, 0); return 1; }
WEAK void PE_SpuDma_WaitForCompletion(void) { host_note("PE_SpuDma_WaitForCompletion", 0, 0, 0, 0); }
WEAK void pe_plat_spu_write_io(uint32_t source, uint32_t size) { host_note("pe_plat_spu_write_io", source, size, 0, 0); }
WEAK void pe_plat_spu_dma_priority_enabled(void) { host_note("pe_plat_spu_dma_priority_enabled", 0, 0, 0, 0); }
WEAK void PE_Decomp_NoteStopDetail(const char *text) { (void)text; host_note("PE_Decomp_NoteStopDetail", 0, 0, 0, 0); }
WEAK int PE_Port_ShouldStop(void) { return 0; }
WEAK void pe_plat_sio_poll_tick(void) { host_note("pe_plat_sio_poll_tick", 0, 0, 0, 0); }   /* the wait's per-poll clock tick (host-only) */
WEAK void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
WEAK _Bool PE_RangeIsRam(pe_addr_t address, size_t size) { return (address & 0x1FFFFFu) + size <= MEM && (address >> 16) != 0x1F80u; }


#ifdef CDX48_REGSTUBS
/* Register-level sets (upload, upload_fault, prepare, callback): the provider is linked with the clean-room 80085E54 adapter
 * (pe_plat2_cdx_spu.c.o), 80085F44 (pe_plat2_spu_b.c.o) and the SPU_DELAY leaves (pe_plat2_spu_a.c.o).  Those objects leave
 * the symbols below undefined; none is reached by these entries, so a call fails the case (reported by name).
 * Build with -DCDX48_REGSTUBS.  The weak stand-ins above (pe_plat_spub_set_callback, pe_plat_cdxspu_spu_write, ...) are replaced
 * by the real objects. */
TRAP(func_80085084) TRAP(PE_Event_Enable) TRAP(PE_Event_Open) TRAP(pe_plat_bios_close_event) TRAP(pe_plat_bios_disable_event)
TRAP(pe_plat_cdxetc_enter_critical) TRAP(pe_plat_cdxetc_exit_critical) TRAP(pe_plat_ctl_counter_setup) TRAP(pe_plat_ctl_counter_start)
TRAP(pe_plat_dma_callback) TRAP(pe_plat_interrupt_callback) TRAP(pe_plat_reset_callback) TRAP(pe_plat_spu_set_reverb_mode_type)
TRAP(PE_Printf_Guest) TRAP(PE_Decomp_GuestTrapDivZero) TRAP(PE_SpuDma_InstallIrq)
/* cdx-spu2: only the reverb work-area clear (not an entry here) reaches this one */
TRAP(PE_Event_ConsumeSpuDma)
#endif
extern void func_800850F4(pe_addr_t src, uint32_t size);
extern void func_80085174(void);
extern void func_800850C0(void) WEAK;   /* the prepare step */
extern void func_80085098(void) WEAK;   /* the DMA-complete callback; a provider that does not define it leaves its vectors unsupported (exit 2) */
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)ret;
    host_log_n = 0;
    if (mutant == 1) return entry == 0x800850F4u || entry == 0x80085174u || entry == 0x80085098u || entry == 0x800850C0u ? 0 : -1;
    if (entry == 0x800850F4u) {
        func_800850F4(a[0], a[1]);
        if (getenv("CDX48_DEBUG_HOST")) { unsigned i; for (i = 0; i < host_log_n; i++) fprintf(stderr, "  host: %s\n", host_log[i]); }
        return 0;
    }
    if (entry == 0x80085174u) { func_80085174(); return 0; }
    if (entry == 0x800850C0u && func_800850C0) { func_800850C0(); return 0; }
    if (entry == 0x80085098u && func_80085098) { func_80085098(); return 0; }
    return -1;
}
