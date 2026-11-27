/* Checker for the cdgen47_stream_deep set (pe_cdx_streamdeep_vectors.py): the CD stream handler 0x8007C564 only.  Scaffold pe_cdx_common.h.
 * One provider object per build, linked as a black box:
 *   -DSD_PUBLIC=1   platform/pe_plat_cd_stream_handler_host.c of the int3 tip (symbol pe_plat_cd_stream_handler_host);
 *                   -DSD_SCRATCH_FRAME=1 / -DSD_MDEC_REPLAY=1 are the two cdgen47 seam mappings (see cdgen47 REPORT section 5);
 *   -DSD_AUTHOR=1   the clean-room provider of sw-cdhost-author (symbol pe_plat_cdhost_stream; seams HostFB_StreamTick, PE_CdReg_ServiceDMA3 host-only,
 *                   pe_plat_cdhost_dma = retail 0x8007CEAC (7 arguments), pe_plat_cd_ready = 0x8007A488).
 * A call of the retail D_800B0CCC callback (sentinel 0x801311C0) is a callee the providers reach through PE_GuestCall / a boundary; a provider that stops there fails the case (trap-call).
 * Provider objects (compiled by pe_cdx_streamdeep_run.sh; $O = scratch output directory, $A = the author's worktree):
 *   $O/public_stream.o = cc -c -std=gnu11 -O2 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/platform/pe_plat_cd_stream_handler_host.c
 *   $O/author_stream.o = cc -c -std=gnu11 -O2 -I$A/pc_port/include -I$A/pc_port/platform -I$A/pc_port/bootstrap $A/pc_port/platform/pe_plat2_cdhost_stream.c
 *   $O/stub_stream.o   = cc -c -std=gnu11 -O2 $O/stub_stream.c   (void pe_plat_cdhost_stream(void) {}  -- the stub baseline: an entry that does nothing)
 * BUILD-public: cc -std=gnu11 -Wall -Wextra -Werror -DSD_PUBLIC=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_streamdeep_check.c $O/public_stream.o -o $O/sd_public
 * BUILD-public-mapped: cc -std=gnu11 -Wall -Wextra -Werror -DSD_PUBLIC=1 -DSD_SCRATCH_FRAME=1 -DSD_MDEC_REPLAY=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_streamdeep_check.c $O/public_stream.o -o $O/sd_public-mapped
 * BUILD-author: cc -std=gnu11 -Wall -Wextra -Werror -DSD_AUTHOR=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_streamdeep_check.c $O/author_stream.o -o $O/sd_author
 * BUILD-author-strict: cc -std=gnu11 -Wall -Wextra -Werror -DSD_AUTHOR=1 -DCDX_STRICT_FRAME_PTR=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_streamdeep_check.c $O/author_stream.o -o $O/sd_author-strict
 * BUILD-stub: cc -std=gnu11 -Wall -Wextra -Werror -DSD_AUTHOR=1 -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tools/pe_cdx_streamdeep_check.c $O/stub_stream.o -o $O/sd_stub
 * Run: build/chk/sd_<name> vectors.txt [noop|flip|retflip|storeflip]   (CDX_CATEGORIES=1, CDX_LINES=1, CDX_TRACE=1, CDX_DEBUG=1) */
#include "pe_cdx_common.h"
#include "pe_plat/mdec_hw.h"

uint8_t PE_CdLoadU8(pe_addr_t a) { return PE_LoadU8(a); }
uint32_t PE_CdLoadU32(pe_addr_t a) { return PE_LoadU32(a); }
void PE_CdStoreU8(pe_addr_t a, uint8_t v) { PE_StoreU8(a, v); }
void PE_CdStoreU32(pe_addr_t a, uint32_t v) { PE_StoreU32(a, v); }
int PE_Port_ShouldStop(void) { return 0; }
void PE_Port_RequestStop(int r) { (void)r; bad_call = 1; trap_name = "PE_Port_RequestStop"; }
CALLEE(pe_plat_cd_ready, 0x8007A488u, 2)
/* DMA3 data effect of the stand-in (the generator's dma model): the channel moves `words` words of the CD data stream, a window of the vector's
 * loads at SD_DMAWIN, to the address the issuer is given; the bytes count as written by the code under test (readable, '@n' write counts). */
#define SD_DMAWIN 0x80180000u
static uint32_t sd_dma_off;
static void sd_dma_effect(uint32_t addr, uint32_t words)
{
    uint32_t n = words > 512u ? 2048u : words * 4u, o;
    if (readable[(SD_DMAWIN + sd_dma_off) & (MEM - 1)] != cur) return;   /* sets without the data window (v1): the DMA stand-in writes nothing, as before */
    if (n > 2048u - sd_dma_off) n = 2048u - sd_dma_off;
    if ((addr & (MEM - 1)) + n > MEM) return;
    for (o = 0; o < n; o += 4) {
        uint32_t d = (addr + o) & (MEM - 1), sidx = (SD_DMAWIN + sd_dma_off + o) & (MEM - 1);
        wr(addr + o, 4); memcpy(ram + d, ram + sidx, 4);
    }
    sd_dma_off += n;
}
#if SD_PUBLIC
TRAP(Bootstrap_ReturnVoid) TRAP(Bootstrap_ReturnVoid4Indirect)
IGNORE(pe_plat_cd_stream_stop) IGNORE(pe_plat_cd_stream_auxiliary)
uint32_t PE_CdReg_DeviceEnabled(void) { return 1; }
CALLEE(pe_plat_cd_data_ready_callback, 0x8007C214u, 0)
void pe_plat_cd_dma_issue_host(uint32_t ch, uint32_t addr, uint32_t blocks, uint32_t words, uint32_t control, uint32_t irq, uint32_t unused)
{ uint32_t a[8]; a[0] = ch; a[1] = addr; a[2] = blocks; a[3] = words; a[4] = control; a[5] = irq; a[6] = unused; a[7] = 0; (void)cdx_calleeN(0x8007CEACu, 7, a); sd_dma_effect(addr, words); }
#if SD_MDEC_REPLAY
void PE_MDEC_GetState(PeMdecState *out) { uint32_t v; memset(out, 0, sizeof *out); if (mmio_read(0x1F801098u, 4, &v)) out->dma1_chcr = v; }
#else
void PE_MDEC_GetState(PeMdecState *out) { memset(out, 0, sizeof *out); }
#endif
void pe_plat_cd_stream_handler_host(void);
#define SD_ENTRY pe_plat_cd_stream_handler_host
#elif SD_AUTHOR
IGNORE(HostFB_StreamTick)
IGNORE(PE_CdReg_ServiceDMA3)
#include "pe_guestcode.h"
/* the same seams as the author's own CDX47_CDHOST stream build: the data-ready callback 0x8007C214 as a recorded callee; a retail jalr through a callback word reaches
 * the guest-hook seam PE_GuestHook_Call(slot, caller, fn, arity, a0..a3, &ret): recorded as callee `fn` with `arity` arguments, result 1 (ran), $v0 = 0;
 * the named refusal (an unresolvable computed jump) is a boundary a faithful run never reaches: a call fails the case */
CALLEE(pe_plat_cd_data_ready_callback, 0x8007C214u, 0)
void PE_GuestHook_Refuse(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3, const char *why)
{ (void)slot; (void)caller; (void)fn; (void)arity; (void)a0; (void)a1; (void)a2; (void)a3; (void)why; bad_call = 1; trap_name = "PE_GuestHook_Refuse"; }
int PE_GuestHook_Call(const char *slot, const char *caller, pe_addr_t fn, unsigned arity, uintptr_t a0, uintptr_t a1, uintptr_t a2, uintptr_t a3, int *ret)
{ (void)slot; (void)caller; (void)cdx_callee(fn, arity, (uint32_t)a0, (uint32_t)a1, (uint32_t)a2, (uint32_t)a3, 0); if (ret) *ret = 0; return 1; }
void pe_plat_cdhost_dma(uint32_t ch, uint32_t addr, uint32_t blocks, uint32_t words, uint32_t control, uint32_t irq, uint32_t unused)
{ uint32_t a[8]; a[0] = ch; a[1] = addr; a[2] = blocks; a[3] = words; a[4] = control; a[5] = irq; a[6] = unused; a[7] = 0; (void)cdx_calleeN(0x8007CEACu, 7, a); sd_dma_effect(addr, words); }
void pe_plat_cdhost_stream(void);
#define SD_ENTRY pe_plat_cdhost_stream
#else
#error "define SD_PUBLIC or SD_AUTHOR"
#endif
static int call(uint32_t entry, const uint32_t *a, uint32_t *ret)
{
    (void)a; (void)ret; if (entry != 0x8007C564u) return -1;
    sd_dma_off = 0;
#if SD_SCRATCH_FRAME
    extra_frame_lo = 0x801FFE00u; extra_frame_hi = 0x801FFF00u;
#endif
#if SD_AUTHOR && !defined(CDX_STRICT_FRAME_PTR)
    /* mapped (default): the clean-room handler's own frame is the fixed host scratch window 0x801FFE80..0x801FFE9F (the stack position of the retail
     * chain; result buffer at +0x10, hand-read FIFO bytes at +8): reads / writes unlogged, pointers into it compared as F+offset, out of the final
     * RAM diff -- the same treatment pe_cdx_common.h gives the slot author's 0x801FFE80..0x801FFE9F scratch window.  -DCDX_STRICT_FRAME_PTR = raw comparison. */
    extra_frame_lo = 0x801FFE80u; extra_frame_hi = 0x801FFEA0u;
#endif
#if (SD_AUTHOR && !defined(CDX_STRICT_FRAME_PTR)) || SD_SCRATCH_FRAME
    /* the original's local frame (stack 0x801FEFE0..0x801FEFFF, its words seeded from the vector's loads) appears at the provider's fixed scratch frame
     * 0x801FFE80..0x801FFE9F: the uninitialised word at frame+8 that the original reads back is then readable by a provider whose frame lies there
     * (the suite seeds 0x55667788 at 0x801FFE88 for the same reason) */
    memcpy(ram + (0x801FFE80u & (MEM - 1)), ram + (0x801FEFE0u & (MEM - 1)), 0x20);
#endif
    if (mutant == 1) return 0;
    SD_ENTRY(); return 0;
}
