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
 */
#include "pe_spu_dma.h"
#include "pe_spu.h"
#include "psx_compat.h"
#include "pe_sdk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GA_D_8009D24C 0x8009D24Cu
#define GA_D_8009B434 0x8009B434u
#define GA_D_8009B44C 0x8009B44Cu
#define GA_D_8009B450 0x8009B450u
#define GA_D_8009B454 0x8009B454u

static uint8_t g_spu_ram[PE_SPU_RAM_SIZE];
static uint16_t g_spu_registers[0x100];
static PeSpuDmaState g_dma;
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
                               uint32_t requested_size, pe_addr_t callback);

int PE_SpuDma_Begin(pe_addr_t source, uint32_t destination,
                    uint32_t requested_size, pe_addr_t callback)
{
    uint32_t dma_size;
    int ok = SpuDma_BeginChecked(source, destination, requested_size, callback);
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

static int SpuDma_BeginChecked(pe_addr_t source, uint32_t destination,
                               uint32_t requested_size, pe_addr_t callback)
{
    uint32_t dma_size;

    if (!g_dma.irq_installed ||
        g_dma.irq_handler != PE_SPU_DMA_IRQ_HANDLER ||
        g_dma.pending ||
        (callback != PE_SPU_DMA_CALLBACK &&
         (callback != 0 || !PE_Event_SpuDmaEnabled())) ||
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
    PE_StoreU32(GA_D_8009B44C, 0);
    PE_StoreU32(GA_D_8009B450, source);
    PE_StoreU32(GA_D_8009B454, dma_size / PE_SPU_DMA_BLOCK_SIZE);
    return 1;
}

int PE_SpuDma_Service(void)
{
    pe_addr_t callback;
    uint32_t first_size;

    if (!g_dma.pending) return 0;

    if (dma_log()) {
        uint32_t now = dma_sum(g_dma.source, g_dma.dma_size);
        fprintf(dma_log(), "COPY src=%08X dst=%05X size=%X sum=%08X%s\n",
                g_dma.source, g_dma.destination, g_dma.dma_size, now,
                now != g_dma_issue_sum ? " SOURCE-CHANGED-SINCE-ISSUE" : "");
        fflush(dma_log());
    }
    if (g_dma.dma_size != 0) {
        first_size = PE_SPU_RAM_SIZE - g_dma.destination;
        if (first_size > g_dma.dma_size) first_size = g_dma.dma_size;
        memcpy(g_spu_ram + g_dma.destination,
               PE_TranslateConst(g_dma.source, first_size), first_size);
        if (first_size != g_dma.dma_size) {
            uint32_t wrap_size = g_dma.dma_size - first_size;
            memcpy(g_spu_ram,
                   PE_TranslateConst(g_dma.source + first_size, wrap_size),
                   wrap_size);
        }
    }
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
        if (PE_Event_DeliverSpuDma()) {
            g_dma.callback_order = ++g_order;
            return 1;
        }
        fprintf(stderr,
                "FATAL: SPU DMA IRQ event-object delivery is unresolved\n");
        abort();
    }
    if (callback != PE_SPU_DMA_CALLBACK) {
        fprintf(stderr,
                "FATAL: SPU DMA IRQ cannot dispatch guest callback 0x%08X\n",
                callback);
        abort();
    }

    func_80085098();
    g_dma.callback_order = ++g_order;
    return 1;
}

void PE_SpuDma_WaitForCompletion(void)
{
    while (PE_LoadU32(GA_D_8009D24C) == 1u) {
        if (!PE_SpuDma_Service()) {
            fprintf(stderr,
                    "FATAL: SPU DMA busy flag has no pending completion event\n");
            abort();
        }
    }
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
