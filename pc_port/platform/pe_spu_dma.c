/*
 * Host model of the SPU DMA channel 4 (asynchronous completion) for Parasite Eve.
 *
 * What this file models (hardware behaviour as documented publicly, psx-spx "DMA Channels" and "SPU Memory Access"):
 *   - the DMA4 register file: MADR (bits 0-23), BCR (block count << 16 | words per block) and CHCR; a CHCR store with the
 *     start bit (bit 24) records the transfer the registers describe (block mode, 16 words per block, direction from bit 0);
 *   - the SPU transfer address register and the SPU register file the guest reads and writes (the audio lane observes
 *     every store, pe_spu.h);
 *   - the 512 KiB SPU RAM image; the 16-bit transfer address counts 8-byte units and wraps at the end of SPU RAM;
 *   - the own-model request recording entry PE_SpuDma_RecordRequest (the host request API PE_SpuDma_Begin / BeginRead
 *     lives in pe_spu_dma_request.c, a test/tool-only TU not linked into the game binary).
 *
 * A request is recorded at issue time.  The data moves in one atomic, deterministic block copy when the controlled event
 * pump PE_SpuDma_Service runs (the host stand-in for the DMA4 interrupt opportunity); it then ends the transfer and
 * delegates the DMA4 completion interrupt handler to the clean-room provider pe_plat_spu_fidma (platform/pe_plat_spu_fidma.c,
 * bound as the guest handler in pe_plat/audio.h; evidence docs/evidence/native-spu-fidma-20261031/): that provider
 * acknowledges the SPU transfer-mode bits and runs the live callback word or raises the SPU DMA event.  This file keeps
 * only the transfer record, the byte movement, the order / interrupt counters and the diagnostics log.  No host pointer
 * is retained or written to guest state.
 */
#include "pe_spu_dma.h"
#include "pe_plat/spu_dma_check.h"
#include "pe_plat_spu_dma_addrs.h"
#include "pe_spu.h"
#include "pe_plat/audio.h"
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

static uint8_t g_spu_ram[PE_SPU_RAM_SIZE];
static uint16_t g_spu_registers[0x100];
static PeSpuDmaState g_dma;
static int g_dma_read;
static uint64_t g_order;
static uint32_t g_dma4_madr, g_dma4_bcr, g_dma4_chcr;   /* DMA4 register file (PE_SpuDma_Dma4Load/Store) */


void PE_SpuDma_Reset(void)
{
    memset(g_spu_ram, 0, sizeof(g_spu_ram));
    memset(g_spu_registers, 0, sizeof(g_spu_registers));
    memset(&g_dma, 0, sizeof(g_dma));
    g_dma_read = 0;
    g_order = 0;
    g_dma4_madr = g_dma4_bcr = g_dma4_chcr = 0;
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
/* Own-model recording entry: validates a request, records it in g_dma and logs the issue.  It leaves no guest-visible
 * bookkeeping word; the host request API (pe_spu_dma_request.c, tests and tools only) layers those on top. */
int PE_SpuDma_RecordRequest(pe_addr_t source, uint32_t destination,
                            uint32_t requested_size, pe_addr_t callback,
                            int read_direction)
{
    uint32_t dma_size;
    FILE *f;

    /* `callback` is recorded only: an issue never reads the live callback slot D_8009B434; the completion handler
     * dispatches whatever the slot holds at that time. */
    f = dma_log();
    /* The acceptance rules and the 64-byte block rounding are the clean-room ladder of pe_plat2_spu_dma_check.c
     * (batch 44a; derived from the spu_dma_hw.h contract, the cdgen48 black-box report and psx-spx). */
    if (!PE_SpuDmaCheck_Request(g_dma.irq_installed, g_dma.irq_handler, g_dma.pending,
                                source, destination, requested_size, read_direction, &dma_size)) {
        if (f && !read_direction) {
            g_dma_issue_sum = 0;
            fprintf(f, "BEGIN REJECT src=%08X dst=%05X req=%X cb=%08X irq=%d/%08X pending=%d sum=%08X\n",
                    source, destination, requested_size, callback,
                    g_dma.irq_installed, g_dma.irq_handler, g_dma.pending, g_dma_issue_sum);
            fflush(f);
        }
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
    if (f && !read_direction) {
        g_dma_issue_sum = dma_sum(source, dma_size);
        fprintf(f, "BEGIN ok src=%08X dst=%05X req=%X size=%X cb=%08X irq=%d/%08X pending=0 sum=%08X\n",
                source, destination, requested_size, dma_size, callback,
                g_dma.irq_installed, g_dma.irq_handler, g_dma_issue_sum);
        fflush(f);
    }
    return 1;
}

/* DMA channel 4 registers (pe_plat/spu_dma_hw.h).  The CHCR start records the transfer the registers
 * describe, with the same fields PE_SpuDma_RecordRequest leaves in g_dma; `source` stays the raw MADR
 * value and the request-size cap of the call API does not apply (the size is BCR's block count). */
static void Dma4Refuse(const char *why)
{
    char msg[200];
    snprintf(msg, sizeof msg, "SPU_DMA/REFUSED DMA4 CHCR start (%s): MADR=0x%08X BCR=0x%08X CHCR=0x%08X",
             why, (unsigned)g_dma4_madr, (unsigned)g_dma4_bcr, (unsigned)g_dma4_chcr);
    fprintf(stderr, "%s (the host SPU DMA model cannot represent this transfer)\n", msg);
    PE_Decomp_NoteStopDetail(msg);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

static void Dma4Start(void)
{
    int read_direction = !(g_dma4_chcr & 1u);
    uint32_t blocks = g_dma4_bcr >> 16, words = g_dma4_bcr & 0xFFFFu;
    uint32_t bytes = blocks * PE_SPU_DMA_BLOCK_SIZE;
    uint32_t destination;
    pe_addr_t registers = PE_LoadU32(0x8009B3FCu);
    pe_addr_t source = g_dma4_madr;
    FILE *f;

    /* the SPU transfer address register (units of 8 bytes) as the guest sees it: the host register file at
     * 0x1F801C00, or a RAM block the register pointer word names (the test fixtures) */
    if ((registers & 0x1FFFFFFFu) == 0x1F801C00u) destination = (uint32_t)g_spu_registers[0x1A6u / 2u] * 8u;
    else if (PE_RangeIsRam(registers, 0x200u)) destination = (uint32_t)PE_LoadU16(registers + 0x1A6u) * 8u;
    else { Dma4Refuse("SPU register block"); return; }
    if (((g_dma4_chcr >> 9) & 3u) != 1u || words * 4u != PE_SPU_DMA_BLOCK_SIZE) { Dma4Refuse("not 16-word block mode"); return; }
    if (!g_dma.irq_installed || g_dma.irq_handler != PE_SPU_DMA_IRQ_HANDLER) { Dma4Refuse("completion handler not installed"); return; }
    if (g_dma.pending) { Dma4Refuse("another transfer pending"); return; }
    if ((source & 3u) != 0) { Dma4Refuse("misaligned source"); return; }
    if (read_direction && bytes == 0) { Dma4Refuse("zero-length read"); return; }
    if (blocks > PE_SPU_RAM_SIZE / PE_SPU_DMA_BLOCK_SIZE || !PE_RangeIsRam(source, bytes) ||
        destination >= PE_SPU_RAM_SIZE) { Dma4Refuse("range"); return; }

    g_dma.pending = 1;
    g_dma_read = read_direction;
    g_dma.source = source;
    g_dma.destination = destination;
    g_dma.requested_size = bytes;
    g_dma.dma_size = bytes;
    g_dma.callback_at_issue = PE_LoadU32(GA_D_8009B434);
    g_dma.data_order = 0;
    g_dma.callback_order = 0;
    f = dma_log();
    if (f && !read_direction) {
        g_dma_issue_sum = dma_sum(source, bytes);
        fprintf(f, "BEGIN ok src=%08X dst=%05X req=%X size=%X cb=%08X irq=%d/%08X pending=0 sum=%08X (DMA4)\n",
                source, destination, bytes, bytes, g_dma.callback_at_issue, g_dma.irq_installed,
                g_dma.irq_handler, g_dma_issue_sum);
        fflush(f);
    }
}

uint32_t PE_SpuDma_Dma4Load(uint32_t index)
{
    if (index == 0) return g_dma4_madr & 0x00FFFFFFu;
    if (index == 1) return g_dma4_bcr;
    return (g_dma4_chcr & ~0x01000000u) | (g_dma.pending ? 0x01000000u : 0u);
}

void PE_SpuDma_Dma4Store(uint32_t index, uint32_t value)
{
    if (index == 0) { g_dma4_madr = value; return; }
    if (index == 1) { g_dma4_bcr = value; return; }
    g_dma4_chcr = value;
    if (value & 0x01000000u) Dma4Start();
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

/* Completion of the transfer in flight (the controlled event pump).  The data moves first, then the transfer ends
 * and the DMA4 completion interrupt handler runs: it is the clean-room provider pe_plat_spu_fidma (platform/
 * pe_plat_spu_fidma.c, bound as retail 0x8007D614 in pe_plat/audio.h), which clears the SPU transfer-mode bits, waits for
 * them to read clear, then dispatches the live callback word or raises the SPU DMA event (an unresolved guest
 * callback is its stop, through the guest hook registry).  This model keeps only the bookkeeping the host tests and the
 * audio lane observe: the order counters (data before callback, no callback order after a stopped dispatch) and the
 * interrupt count.  Returns 1 if an interrupt ran (also when its callback requested a stop), 0 if nothing was pending. */
int PE_SpuDma_Service(void)
{
    if (!g_dma.pending) return 0;
    SpuDma_CopyBytes(g_dma.source, g_dma.destination, g_dma.dma_size, g_dma_read);
    if (dma_log() && !g_dma_read) {
        uint32_t now = dma_sum(g_dma.source, g_dma.dma_size);
        fprintf(dma_log(), "COPY src=%08X dst=%05X size=%X sum=%08X%s\n", g_dma.source, g_dma.destination,
                g_dma.dma_size, now, now != g_dma_issue_sum ? " SOURCE-CHANGED-SINCE-ISSUE" : "");
        fflush(dma_log());
    }
    g_dma.pending = 0;
    g_dma.data_order = ++g_order;
    g_dma.event_count++;
    pe_plat_spu_fidma();
    if (!PE_Port_ShouldStop()) g_dma.callback_order = ++g_order;
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
