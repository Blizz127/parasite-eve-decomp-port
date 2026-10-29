/*
 * Phase 6E-B48A — asynchronous SPU DMA4 completion for Parasite Eve.
 *
 * Retail evidence represented here:
 *
 *   func_8007D9F8 (0x8007D9F8..0x8007DA7B, 33 words) selects its mode-0
 *   DMA path, programs the SPU transfer address/control and DMA4, then
 *   returns the requested size without polling DMA4 completion.
 *
 *   DMA4 MADR 0x1F8010C0 = guest source
 *   DMA4 BCR  0x1F8010C4 = (ceil(size / 64) << 16) | 0x10
 *   DMA4 CHCR 0x1F8010C8 = 0x01000201
 *   DPCR       0x1F801014 = (old & 0xF0FFFFFF) | 0x20000000
 *   SPU TSA    0x1F801DA6 = destination >> D_8009B424 (16-bit)
 *   SPU CTRL   0x1F801DAA = (old & 0xFFCF) | 0x20 (16-bit)
 *
 * func_8007DD14 registers func_8007D614 in DMA callback slot 4.  The
 * libetc DMA dispatcher enables DICR 0x1F8010F4 master bit 23 and channel-4
 * enable bit 20, and observes/acknowledges channel-4's bit 28.  I_STAT
 * 0x1F801070/I_MASK 0x1F801074 belong to the surrounding kernel interrupt
 * dispatcher; this SPU leaf chain does not access them directly.
 *
 *   func_8007D614 (0x8007D614..0x8007D6CF, 47 words) is the DMA4 IRQ
 *   callback.  After clearing/polling SPU transfer control, it reads and
 *   invokes D_8009B434.  func_80085098 then deregisters that callback and
 *   stores zero to D_8009D24C.
 *
 * The host therefore records the request at issue time and performs one
 * atomic, deterministic DMA block transfer only when the controlled event
 * pump runs.  The 16-bit TSA wraps at the 512 KiB SPU-RAM boundary.  No host
 * pointer is retained or written to guest state.
 *
 * Completion follows func_8007D614's two arms on the LIVE slot D_8009B434.
 * Retail reads the slot in the IRQ and never at issue, so issue does not
 * check it.  The identity func_80085098 runs natively.  Any other value is
 * retail's jalr at 0x8007D6A4, taken through the guest-code registry with
 * $a0 = 0xF0000000 (the lui in the 0x8007D694 delay slot; $a1..$a3 hold the
 * DMA dispatcher's values and are not reproduced).  With no native code at
 * that address it is an indirect boundary that stops.  A null slot is
 * DeliverEvent(0xF0000009, 0x20): the BIOS marks an enabled event of that
 * class delivered and ignores one that is not enabled.  SpuInit's NOINTR,
 * handler-free open at 0x8007D19C is the only OpenEvent of that class on
 * disc 1, and PE_Event_Open tracks it, so an event that is not enabled is
 * the retail no-op (docs/evidence/native-spudma-20261026/REPORT.md).
 */
#include "pe_spu_dma.h"
#include "pe_spu.h"
#include "psx_compat.h"
#include "pe_sdk.h"
#include "game_port.h"
#include "pe_bootstrap.h"
#include "pe_guestcode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void PE_Decomp_NoteStopDetail(const char *text);   /* pe_guest_decomp.c */

#define GA_D_8009D24C 0x8009D24Cu
#define GA_D_8009B434 0x8009B434u
#define SPU_DMA_JALR_A0 0xF0000000u   /* $a0 at func_8007D614's jalr */
#define GA_D_8009B44C 0x8009B44Cu
#define GA_D_8009B450 0x8009B450u
#define GA_D_8009B454 0x8009B454u

static uint8_t g_spu_ram[PE_SPU_RAM_SIZE];
static uint16_t g_spu_registers[0x100];
static PeSpuDmaState g_dma;
static int g_dma_read;
static uint64_t g_order;

/* Translated retail completion callback, implemented with the wrapper. */
extern void func_80085098(void);

static int RoundDmaSize(uint32_t requested_size, uint32_t *dma_size)
{
    uint64_t rounded;

    if (!dma_size) return 0;
    if (requested_size == 0) {
        *dma_size = 0;
        return 1;
    }
    rounded = ((uint64_t)requested_size +
               (PE_SPU_DMA_BLOCK_SIZE - 1u)) &
              ~(uint64_t)(PE_SPU_DMA_BLOCK_SIZE - 1u);
    if (rounded > UINT32_MAX) return 0;
    *dma_size = (uint32_t)rounded;
    return 1;
}

void PE_SpuDma_Reset(void)
{
    memset(g_spu_ram, 0, sizeof(g_spu_ram));
    memset(g_spu_registers, 0, sizeof(g_spu_registers));
    memset(&g_dma, 0, sizeof(g_dma));
    g_dma_read = 0;
    g_order = 0;
    PE_Spu_Reset();                          /* audio lane (pe_spu.h) */
}

int PE_SpuDma_InstallIrq(pe_addr_t handler)
{
    if (handler != PE_SPU_DMA_IRQ_HANDLER) return 0;
    g_dma.irq_installed = 1;
    g_dma.irq_handler = handler;
    return 1;
}

/* Audio diagnostics: PE_AUDIO_DMA_LOG=path logs every SPU DMA request
 * (accepted or rejected) and the copy, with a source checksum at issue and
 * at copy time (a mismatch means the guest reused the buffer first). */
static FILE *dma_log(void)
{
    static FILE *f;
    static int init;
    if (!init) {
        const char *p = getenv("PE_AUDIO_DMA_LOG");
        init = 1;
        if (p && *p) f = fopen(p, "w");
    }
    return f;
}

static uint32_t dma_sum(pe_addr_t src, uint32_t n)
{
    uint32_t h = 2166136261u;
    if (!PE_RangeIsRam(src, n)) return 0;
    const uint8_t *b = PE_TranslateConst(src, n);
    for (uint32_t i = 0; i < n; i++) h = (h ^ b[i]) * 16777619u;
    return h;
}

static uint32_t g_dma_issue_sum;
static int SpuDma_BeginChecked(pe_addr_t source, uint32_t destination,
                               uint32_t requested_size, pe_addr_t callback,
                               int read_direction);

int PE_SpuDma_Begin(pe_addr_t source, uint32_t destination,
                    uint32_t requested_size, pe_addr_t callback)
{
    uint32_t dma_size;
    int ok = SpuDma_BeginChecked(source, destination, requested_size, callback, 0);
    FILE *f = dma_log();
    if (f) {
        dma_size = 0;
        (void)RoundDmaSize(requested_size, &dma_size);
        g_dma_issue_sum = ok ? dma_sum(source, dma_size) : 0;
        fprintf(f, "BEGIN %s src=%08X dst=%05X req=%X size=%X cb=%08X irq=%d/%08X pending=%d sum=%08X\n",
                ok ? "ok" : "REJECT", source, destination, requested_size,
                dma_size, callback, g_dma.irq_installed, g_dma.irq_handler,
                ok ? 0 : g_dma.pending, g_dma_issue_sum);
        fflush(f);
    }
    return ok;
}

int PE_SpuDma_BeginRead(pe_addr_t guest_destination, uint32_t spu_source,
                        uint32_t requested_size, pe_addr_t callback)
{
    if (requested_size == 0u)
        return 0;
    return SpuDma_BeginChecked(guest_destination, spu_source, requested_size,
                               callback, 1);
}

static int SpuDma_BeginChecked(pe_addr_t source, uint32_t destination,
                               uint32_t requested_size, pe_addr_t callback,
                               int read_direction)
{
    uint32_t dma_size;

    /* `callback` is recorded only: func_8007D9F8 never reads D_8009B434, and
     * func_8007D614 dispatches whatever the slot holds at completion. */
    if (!g_dma.irq_installed ||
        g_dma.irq_handler != PE_SPU_DMA_IRQ_HANDLER ||
        g_dma.pending ||
        (source & 3u) != 0 ||
        (destination & 7u) != 0 ||
        requested_size > PE_SPU_DMA_MAX_REQUEST ||
        !RoundDmaSize(requested_size, &dma_size) ||
        !PE_RangeIsRam(source, dma_size) ||
        destination >= PE_SPU_RAM_SIZE ||
        dma_size > PE_SPU_RAM_SIZE) {
        return 0;
    }

    g_dma.pending = 1;
    g_dma_read = read_direction;
    g_dma.source = source;
    g_dma.destination = destination;
    g_dma.requested_size = requested_size;
    g_dma.dma_size = dma_size;
    g_dma.callback_at_issue = callback;
    g_dma.data_order = 0;
    g_dma.callback_order = 0;

    /* Exact guest-visible bookkeeping from func_8007D778's mode-0
     * operation-1/operation-3 sequence.  BCR's low half is the fixed
     * 0x10 words-per-block; D_8009B454 stores its upper-half block count. */
    PE_StoreU32(GA_D_8009B44C, read_direction ? 1u : 0u);
    PE_StoreU32(GA_D_8009B450, source);
    PE_StoreU32(GA_D_8009B454, dma_size / PE_SPU_DMA_BLOCK_SIZE);
    return 1;
}

/* 7D6A4: retail's jalr through the live slot.  Only a native thunk of at
 * most four parameters can be called with what the site leaves in $a0..$a3;
 * anything else is an indirect boundary that stops, named in the stop report
 * like the registry's own boundaries.  Returns 0 after that stop. */
static int SpuDma_CallGuest(pe_addr_t callback)
{
    const PeGuestCodeEntry *e = PE_GuestCode_Resolve(callback);
    char msg[200];

    if (e && e->thunk) {
        (void)PE_GuestCall("D_8009B434 (SPU DMA callback)", callback, 0u,
                           SPU_DMA_JALR_A0, 0u, 0u, 0u);
        return 1;
    }
    snprintf(msg, sizeof msg,
             "CPU_BOUNDARY/REFUSED pc=0x%08X ra=func_8007D614 sym=D_8009B434 "
             "arity=0 a0=0x%08X a1=0x00000000 a2=0x00000000 a3=0x00000000",
             (unsigned)callback, (unsigned)SPU_DMA_JALR_A0);
    fprintf(stderr, "%s (native only: no C for this SPU DMA callback; "
            "not emulated)\n", msg);
    Bootstrap_ReturnVoid4Indirect("D_8009B434", "func_8007D614", callback,
                                  SPU_DMA_JALR_A0, 0u, 0u, 0u);
    PE_Decomp_NoteStopDetail(msg);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}

/* Native byte movement only. Caller has checked guest RAM, SPU range and
 * size. This helper contains no SDK callback or guest bookkeeping code. */
static void SpuDma_CopyBytes(pe_addr_t guest, uint32_t address, uint32_t size,
                             int read_direction)
{
    uint32_t first_size;
    if (size != 0) {
        first_size = PE_SPU_RAM_SIZE - address;
        if (first_size > size) first_size = size;
        if (read_direction) {
            memcpy(PE_Translate(guest, first_size),
                   g_spu_ram + address, first_size);
            if (first_size != size) {
                uint32_t wrap_size = size - first_size;
                memcpy(PE_Translate(guest + first_size, wrap_size),
                       g_spu_ram, wrap_size);
            }
        } else {
            memcpy(g_spu_ram + address,
                   PE_TranslateConst(guest, first_size), first_size);
            if (first_size != size) {
                uint32_t wrap_size = size - first_size;
                memcpy(g_spu_ram,
                       PE_TranslateConst(guest + first_size, wrap_size),
                       wrap_size);
            }
        }
        PE_Spu_NotifyRamTransfer(address, size);
    }
}

int PE_SpuDma_Service(void)
{
    pe_addr_t callback;

    if (!g_dma.pending) return 0;

    if (dma_log() && !g_dma_read) {
        uint32_t now = dma_sum(g_dma.source, g_dma.dma_size);
        fprintf(dma_log(), "COPY src=%08X dst=%05X size=%X sum=%08X%s\n",
                g_dma.source, g_dma.destination, g_dma.dma_size, now,
                now != g_dma_issue_sum ? " SOURCE-CHANGED-SINCE-ISSUE" : "");
        fflush(dma_log());
    }
    SpuDma_CopyBytes(g_dma.source, g_dma.destination, g_dma.dma_size, g_dma_read);
    /* Keep the physical SPU's FIFO cursor coherent with DMA completion. */
    if ((PE_LoadU32(0x8009B3FCu) & 0x1FFFFFFFu) == 0x1F801C00u)
        PE_Spu_SetTransferCursor(g_dma.destination + g_dma.dma_size);
    g_dma.data_order = ++g_order;

    /* DMA4 is no longer active before func_8007D614 dispatches the
     * application callback.  The IRQ handler reads the live guest slot. */
    g_dma.pending = 0;
    g_dma.event_count++;
    /* Original DMA IRQ clears transfer-control bits before dispatch. */
    pe_addr_t registers = PE_LoadU32(0x8009B3FCu);
    if ((registers & 0x1FFFFFFFu) == 0x1F801C00u)
        PE_SpuRegister_StoreU16(0x1AAu, PE_SpuRegister_LoadU16(0x1AAu) & 0xFFCFu);
    else if (PE_RangeIsRam(registers, 0x200u))
        PE_StoreU16(registers + 0x1AAu, PE_LoadU16(registers + 0x1AAu) & 0xFFCFu);
    callback = PE_LoadU32(GA_D_8009B434);
    if (callback == 0) {
        /* 7D6B4: DeliverEvent(0xF0000009, 0x20), a no-op unless enabled. */
        (void)PE_Event_DeliverSpuDma();
    } else if (callback == PE_SPU_DMA_CALLBACK) {
        func_80085098();
    } else if (!SpuDma_CallGuest(callback)) {
        return 1;                   /* the data moved; the IRQ tail stopped */
    }
    g_dma.callback_order = ++g_order;
    return 1;
}

/* Retail func_80085174 spins while D_8009D24C == 1 until the DMA4 IRQ's
 * callback clears it.  The host has no asynchronous IRQ, so each turn
 * services the pending transfer.  A stop, requested earlier or by that
 * service's callback dispatch, ends the wait with the flag still set; the
 * callers then return without further guest writes.  A set flag with no
 * transfer pending is a state retail can only spin in, because no DMA4 IRQ
 * will come, so it stops instead of aborting. */
void PE_SpuDma_WaitForCompletion(void)
{
    while (PE_LoadU32(GA_D_8009D24C) == 1u) {
        if (PE_Port_ShouldStop()) return;
        if (!PE_SpuDma_Service()) {
            static const char msg[] =
                "SPU_DMA/STALL func_80085174: D_8009D24C=1 with no DMA4 "
                "transfer pending";
            fprintf(stderr, "%s (retail spins until a DMA4 interrupt the "
                    "host model has no transfer for)\n", msg);
            PE_Decomp_NoteStopDetail(msg);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return;
        }
    }
}

int PE_SpuDma_DeliverPending(void)
{
    if (PE_Port_ShouldStop()) return 0;
    if (!g_dma.pending) return 1;
    (void)PE_SpuDma_Service();
    return !PE_Port_ShouldStop();
}

void PE_SpuDma_GetState(PeSpuDmaState *out)
{
    if (out) *out = g_dma;
}

pe_addr_t PE_SpuDma_ReadMADR(void)
{
    return g_dma.source;
}

uint8_t PE_SpuRam_LoadU8(uint32_t address)
{
    if (address >= PE_SPU_RAM_SIZE) {
        fprintf(stderr, "FATAL: SPU RAM address 0x%08X is out of range\n",
                address);
        abort();
    }
    return g_spu_ram[address];
}

uint16_t PE_SpuRegister_LoadU16(uint32_t offset)
{
    uint16_t live;
    if ((offset & 1u) || offset >= 0x200u) abort();
    if (offset == 0x1AEu)
        return PE_Spu_CaptureStatus(g_spu_registers[offset / 2u],
                                   g_spu_registers[0x1ACu / 2u]);
    /* Audio lane: hardware-live registers (ENDX, envelope level) when the
     * synthesizer is active (pe_spu.h); plain storage otherwise. */
    if (PE_Spu_OnRegisterRead(offset, &live)) return live;
    return g_spu_registers[offset / 2u];
}

void PE_SpuRegister_StoreU16(uint32_t offset, uint16_t value)
{
    if ((offset & 1u) || offset >= 0x200u) abort();
    g_spu_registers[offset / 2u] = value;
    PE_Spu_OnRegisterWrite(offset, value);   /* audio lane (pe_spu.h) */
}

uint8_t *PE_SpuRam_Data(void)
{
    return g_spu_ram;
}
