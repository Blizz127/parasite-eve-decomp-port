/* Host-side interrupt delivery: the DICR rising-edge bridge, the pending-IRQ
 * service entry, the I_STAT/DICR write observers and the callback routing the
 * clean-room guest handlers use.  This file is written clean-room from the
 * port's IRQ delivery contract (pe_plat/irq_dispatch.h), its host callers and
 * tests, psx-spx and the black-box IRQ vector sets cdgen43; no SDK source,
 * decompiled source, tables or disassembly were read.
 *
 * Division of labour (see pe_plat/irq_callback.h):
 *   - what the guest-visible CPU interrupt dispatcher (func_80073F00) and the
 *     DMA interrupt handler (func_80074520) do is implemented once, in
 *     pe_plat_irq_callback.c (pe_plat_irq_dispatch / pe_plat_dma_handler).
 *     Nothing here re-implements their walks, acknowledgements or watchdog.
 *   - this file supplies the host framing around them: a generation check,
 *     the critical-section / masked-status gates, a stop-epoch comparison, the
 *     one-shot DICR-edge-to-I_STAT bridge, and the routing of the guest
 *     callbacks that those two handlers jump through.
 *
 * Hardware facts used (psx-spx "Interrupts" and "DMA Channels"):
 *   - I_STAT/I_MASK are 16-bit; I_STAT is acknowledged by writing 0 to a bit.
 *   - DICR bit 31 is the master flag, derived from bit 15 (force), or from
 *     (master enable bit 23 AND any enabled flag, bits 16-22 & 24-30); a
 *     false->true change of that bit is what raises I_STAT source 3 (DMA).
 *     DICR flag bits 24-30 are acknowledged by writing 1.
 *
 * The observers (PE_IRQ_ObserveIstatWrite / PE_IRQ_ObserveDicrWrite) are
 * called synchronously by the MMIO write path after every I_STAT and DICR
 * store.  They do two things: keep the value-only evidence record, and note
 * which source or channel was just acknowledged.  The handlers acknowledge
 * and then jump to the registered word, so "the next callback whose address
 * equals that source's/channel's slot" is the one the handler is serving;
 * that is how a plain pe_plat_invoke_guest_callback(fn) call is attributed
 * to the CPU table or the DMA table without the handlers having to say so. */
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "pe_irq_delivery.h"
#include "pe_irq.h"
#include "pe_plat/irq_callback.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "pe_gpu.h"
#include "pe_mdec.h"
#include "pe_cdreg.h"
#include "pe_spu_dma.h"
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_bootstrap.h"
#include "game_port.h"

/* ---- guest RAM words the host framing reads (the same words the clean-room
 * dispatcher and DMA handler use; addresses from irq_callback.c's header and
 * the tests: ready halfword, CPU slot table, DMA callback table, the DICR
 * pointer word passed to DMA callbacks in $a0). */
#define G_READY        0x800945E4u  /* halfword: non-zero once ResetCallback ran */
#define G_CPU_SLOTS    0x800945E8u  /* CPU source n handler at +4n (n = 0..10) */
#define G_CPU_SOURCES  11u
#define G_DMA_SLOTS    0x800956C0u  /* DMA channel n callback at +4n (n = 0..6 used) */
#define G_DMA_CHANNELS 7u
#define G_DMA_ARG      0x800956BCu  /* word handed to a DMA callback as $a0 (the DICR address) */

/* ---- hardware bit layout ---- */
#define DICR_FLAG_SHIFT 24u
#define DICR_FLAG_MASK  0x7F000000u
#define IRQ_SRC_DMA     0x0008u     /* I_STAT bit 3 */

/* ---- guest code identities the host runs natively (census "handled" lists:
 * tools/analysis/hook_writer_census.py, rows dma_table and cpu_table) ---- */
#define H_VSYNC_HANDLER 0x8007440Cu /* CPU source 0 default: pe_plat_vsync_handler */
#define H_DMA_HANDLER   0x80074520u /* CPU source 3 default: pe_plat_dma_handler */
#define H_CD_POLL       0x8007C13Cu /* CPU source 2: libcd interrupt poll */
#define H_DMA_PUMP      0x80076EE4u /* DMA channel 2: GPU idle queue pump */
#define H_DMA_CD_DATA   0x8007C214u /* DMA channel 3: CD data-ready callback */
#define H_DMA_OVL_A     0x801214D4u /* DMA channel 0/1: room-overlay MDEC callbacks */
#define H_DMA_OVL_B     0x80191DC8u
/* Callbacks whose C exists but whose state a host model owns: SpuInit's DMA4
 * completion tail and DsRead's DMA3 completion.  The DMA path refuses them. */
#define H_DMA_OWNED_SPU 0x8007D614u
#define H_DMA_OWNED_CD  0x80081110u

/* Hook-site names; the census marks the dispatching lines by these strings. */
#define SITE_DMA_CALL  "func_80074520_dma_indirect_call"
#define SITE_DMA_OWNED "func_80074520_model_owned_callback"
#define SITE_CPU_CALL  "func_80073F00_cpu_indirect_call"
#define SITE_GUARD     "func_80074384"   /* ReturnFromException wrapper: no host counterpart */

/* ---- evidence record and attribution state ---- */
static PeIrqDeliveryTrace g_trace;
static uint64_t g_order;            /* one monotonic clock for every recorded event */

typedef struct {
    int valid;                      /* an acknowledgement was seen and not yet consumed */
    uint32_t index;                 /* source (CPU) or channel (DMA) */
    PeIrqGeneration generation;     /* a host reset invalidates the note */
} PeAckNote;
static PeAckNote g_cpu_note, g_dma_note;

static void note_ack(PeAckNote *n, uint32_t index)
{
    n->valid = 1;
    n->index = index;
    n->generation = PE_IRQ_Generation();
}

/* A note still describes the callback `fn` when it is current and the slot of
 * the noted index holds exactly `fn`. */
static int note_matches(const PeAckNote *n, uint32_t table, uint32_t limit, uint32_t fn)
{
    return n->valid && n->generation == PE_IRQ_Generation() &&
           n->index < limit && PE_LoadU32(table + 4u * n->index) == fn;
}

void PE_IRQ_DeliveryTraceReset(void)
{
    memset(&g_trace, 0, sizeof g_trace);
    g_order = 0u;
    memset(&g_cpu_note, 0, sizeof g_cpu_note);
    memset(&g_dma_note, 0, sizeof g_dma_note);
}

void PE_IRQ_GetDeliveryTrace(PeIrqDeliveryTrace *out)
{
    if (out != NULL) *out = g_trace;
}

/* ---- DICR rising edge -> I_STAT source 3 ---- */
PeIrqEdgeResult PE_IRQ_BridgeDICRRisingEdge(PeIrqGeneration generation)
{
    g_trace.edge_bridge_calls++;
    if (generation != PE_IRQ_Generation()) return PE_IRQ_EDGE_STALE;   /* edge stays latched */
    if (!PE_GPU_TakeDICRRisingEdge()) return PE_IRQ_EDGE_NONE;
    if (!PE_IRQ_AssertSourcesForGeneration(IRQ_SRC_DMA, generation)) return PE_IRQ_EDGE_STALE;
    g_trace.edge_assert_order = ++g_order;
    return PE_IRQ_EDGE_ASSERTED;
}

/* ---- CPU interrupt service ---- */
PeIrqServiceResult PE_IRQ_ServicePendingForGeneration(PeIrqGeneration generation)
{
    unsigned epoch;
    int was_ready;

    g_trace.cpu_service_calls++;
    if (generation != PE_IRQ_Generation()) return PE_IRQ_SERVICE_STALE;
    /* Interrupts are held off inside a critical section, and a source that is
     * not enabled in I_MASK stays pending in I_STAT: both leave everything
     * untouched for a later opportunity. */
    if (PE_Irq_LockDepth() > 0) return PE_IRQ_SERVICE_RETURNED;
    if ((uint16_t)(PE_IRQ_ReadStatus() & PE_IRQ_GetMask()) == 0u) return PE_IRQ_SERVICE_RETURNED;

    epoch = PE_Port_StopEpoch();
    was_ready = PE_LoadU16(G_READY) != 0u;
    g_trace.cpu_service_entries++;
    (void)pe_plat_irq_dispatch();
    if (!was_ready) {
        /* Before the controller is initialised the guest dispatcher only
         * reports the interrupt and then leaves through ReturnFromException,
         * which the host does not have: name that exit, report a boundary. */
        Bootstrap_ReturnVoid(SITE_GUARD, "func_80073F00");
        return PE_IRQ_SERVICE_BOUNDARY;
    }
    /* A stop requested while serving (an unresolved callback, a nested
     * boundary) ends the pass early; the pre-existing stop state does not. */
    return PE_Port_StopEpoch() == epoch ? PE_IRQ_SERVICE_RETURNED : PE_IRQ_SERVICE_BOUNDARY;
}

/* ---- write observers ---- */
void PE_IRQ_ObserveIstatWrite(uint16_t value)
{
    uint32_t cleared = (uint32_t)(uint16_t)~value;   /* bits written as 0 = acknowledged */
    uint32_t src;

    /* One source acknowledged by itself is a dispatcher acknowledgement;
     * wider clears (reset, stop, watchdog) are not attributed to a source. */
    if (cleared == 0u || (cleared & (cleared - 1u)) != 0u) return;
    src = (uint32_t)__builtin_ctz(cleared);
    g_trace.last_cpu_source = src;
    g_trace.last_cpu_ack_write = value;
    g_trace.last_cpu_handler = src < G_CPU_SOURCES ? PE_LoadU32(G_CPU_SLOTS + 4u * src) : 0u;
    if (g_trace.cpu_ack_count < sizeof g_trace.cpu_ack_sources)
        g_trace.cpu_ack_sources[g_trace.cpu_ack_count] = (uint8_t)src;
    g_trace.cpu_ack_count++;
    g_trace.cpu_ack_order = ++g_order;
    note_ack(&g_cpu_note, src);
}

void PE_IRQ_ObserveDicrWrite(uint32_t value)
{
    uint32_t flags = value & DICR_FLAG_MASK;   /* ones written to flag bits acknowledge them */
    uint32_t ch;

    if (flags == 0u) return;
    g_trace.last_dma_ack_write = value;
    g_trace.dma_ack_order = ++g_order;
    for (ch = 0u; ch < 7u; ch++) {
        if (!(flags & (1u << (DICR_FLAG_SHIFT + ch)))) continue;
        g_trace.last_dma_channel = ch;
        g_trace.last_dma_handler = PE_LoadU32(G_DMA_SLOTS + 4u * ch);
        if (g_trace.dma_ack_count < sizeof g_trace.dma_ack_channels)
            g_trace.dma_ack_channels[g_trace.dma_ack_count] = (uint8_t)ch;
        g_trace.dma_ack_count++;
        note_ack(&g_dma_note, ch);
    }
}

/* ---- callback routing ----
 * Both services return 1 when they own the callback (with *ok = 1 only when
 * it completed with no new stop) and 0 when `fn` is not one they serve, in
 * which case the generic guest-callback path runs it. */

/* The DMA channel callback the DMA handler is about to run.  The retail jump
 * passes the DICR address word in $a0 and nothing else. */
int PE_IRQ_DmaCallbackService(uint32_t fn, int *ok)
{
    unsigned epoch;
    uint32_t arg0;
    int result = 0, ran;

    if (!note_matches(&g_dma_note, G_DMA_SLOTS, G_DMA_CHANNELS, fn)) return 0;
    g_dma_note.valid = 0;
    g_trace.dma_callback_order = ++g_order;
    g_trace.last_dma_handler = fn;
    g_trace.dma_callback_istat = PE_IRQ_ReadStatus();
    g_trace.dma_callback_dicr = PE_GPU_ReadDICR();
    g_trace.dma_callback_chcr = PE_GPU_ReadDMA2CHCR();

    epoch = PE_Port_StopEpoch();
    arg0 = PE_LoadU32(G_DMA_ARG);
    switch (fn) {
    case H_DMA_PUMP:
        ran = 0;
        (void)PE_func_80076EE4_Pump(&ran);      /* ran = 0 when a nested boundary did not return */
        result = ran;
        break;
    case H_DMA_CD_DATA:
        func_8007C214();
        result = 1;
        break;
    case H_DMA_OVL_A:
        func_801214D4();
        result = 1;
        break;
    case H_DMA_OVL_B:
        func_80191DC8();
        result = 1;
        break;
    case H_DMA_OWNED_SPU:
    case H_DMA_OWNED_CD:
        PE_GuestHook_Refuse(SITE_DMA_OWNED, "func_80074520", fn, 0u, arg0, 0u, 0u, 0u,
                            "a host model owns this DMA completion; its C must not run here");
        break;
    default:
        result = PE_GuestHook_Call(SITE_DMA_CALL, "func_80074520", fn, 0u,
                                   arg0, 0u, 0u, 0u, NULL);
        break;
    }
    *ok = result && PE_Port_StopEpoch() == epoch;
    return 1;
}

/* The CPU interrupt table callback the dispatcher is about to run.  Retail
 * handlers take no arguments from this site. */
int PE_IRQ_CpuCallbackService(uint32_t fn, int *ok)
{
    unsigned epoch;
    int result = 0;

    if (!note_matches(&g_cpu_note, G_CPU_SLOTS, G_CPU_SOURCES, fn)) return 0;
    g_cpu_note.valid = 0;
    g_dma_note.valid = 0;        /* a new CPU callback starts a fresh DMA scan */
    g_trace.cpu_callback_order = ++g_order;
    g_trace.last_cpu_handler = fn;

    epoch = PE_Port_StopEpoch();
    switch (fn) {
    case H_DMA_HANDLER:
        g_trace.dma_dispatch_entries++;
        (void)pe_plat_dma_handler();
        result = 1;
        break;
    case H_VSYNC_HANDLER:
        (void)pe_plat_vsync_handler();
        result = 1;
        break;
    case H_CD_POLL:
        func_8007C13C();
        result = 1;
        break;
    default:
        result = PE_GuestHook_Call(SITE_CPU_CALL, "func_80073F00", fn, 0u,
                                   0u, 0u, 0u, 0u, NULL);
        break;
    }
    *ok = result && PE_Port_StopEpoch() == epoch;
    return 1;
}

/* ---- DMA diagnostic support ----
 * The DMA handler's force-bit diagnostic prints the live MADR of channels
 * 0..6 in order; this supplies them from the host models that own each
 * channel (MDEC 0/1, GPU 2 and 6, CD 3, SPU 4; channel 5 has no model).
 * Reading channel 0 opens a diagnostic record. */
uint32_t pe_plat_read_dma_madr(uint32_t channel)
{
    uint32_t value = 0u;
    PeMdecState mdec;

    switch (channel) {
    case 0u:
        PE_MDEC_GetState(&mdec);
        value = mdec.dma0_madr;
        break;
    case 1u:
        PE_MDEC_GetState(&mdec);
        value = mdec.dma1_madr;
        break;
    case 2u: value = PE_GPU_ReadDMA2MADR(); break;
    case 3u: value = PE_CdReg_ReadU32(PE_CDREG_DMA3); break;
    case 4u: value = PE_SpuDma_ReadMADR(); break;
    case 6u: value = PE_GPU_ReadDMA6Register(0u); break;
    default: break;
    }
    if (channel == 0u) {
        g_trace.dma_diagnostic_count++;
        g_trace.dma_diagnostic_dicr = PE_GPU_ReadDICR();
    }
    if (channel < 7u) g_trace.dma_diagnostic_madr[channel] = value;
    return value;
}
