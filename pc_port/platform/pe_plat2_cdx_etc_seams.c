/* Host seams of the libetc family (cdx-etc): critical-section depth, source-0 BIOS diagnostics snapshot,
 * the BIOS event table, and the host/test reset owner PE_Sdk_ResetState.
 *
 * Host contracts written clean-room from the port's consumers and tests; no original or translated code
 * read.  Sources of each contract:
 *  - lock depth: pe_sdk.h ("diagnostic: current critical depth"), the delivery gate in the IRQ provider
 *    (host framing: a critical section defers pending CPU interrupts), tests DAY1_blank_edges (n nested
 *    Enters need n Exits before a pending VBlank is dispatched) and the "unbalanced Enter/Exit" asserts;
 *  - source-0 snapshot: the PeIrqSource0BiosState layout in pe_sdk.h and tests B53I_B1_source0_bios_order,
 *    B53I_B1_source3_setter_guest_authority, B53I_B1_same_handler_reset_recovery, DAY1_blank_edges;
 *  - events: psx-spx "BIOS Event Functions" (event control block: class, spec, mode 1000h = handler /
 *    2000h = mark ready, status disabled / enabled-busy / ready; Test returns 1 once for a ready event and
 *    switches it back to busy) plus the consumer contracts in pe_sdk.h and tests DAY2_spu_dma_event,
 *    BIOS_deliver_event_and_err_math, BIOS_event_services, ATK19_audio_timer_preserves_battle_record,
 *    SPU_native_event_install and 409B4_card_init;
 *  - reset: the PE_Sdk_ResetState contract in pe_sdk.h and the tests that call it. */
#include <stdint.h>
#include <string.h>

#define PE_CDXETC_HOST_SEAMS 1   /* keep the raw pe_plat_bios_* names in this TU */
#include "pe_sdk.h"
#include "pe_audio_driver.h"
#include "pe_mmio.h"
#include "pe_sio0.h"
#include "pe_guestcode.h"
#include "pe_plat/cdx_etc.h"
#include "pe_plat/cdx_etc_seams.h"
#include "pe_plat/irq_hw.h"
#include "pe_plat/irq_dispatch.h"
#include "pe_plat/spu_dma_hw.h"
#include "pe_plat/timer1_hw.h"
#include "pe_plat/rcnt2_hw.h"
#include "pe_plat/cd_regs.h"
#include "pe_plat/gpu_hw.h"
#include "pe_plat/mdec_hw.h"
#include "game_port.h"

/* ── critical-section depth ─────────────────────────────────────────── */

static int lock_depth;

int pe_plat_bios_enter_critical(void)
{
    lock_depth++;
    return 0;   /* the veneer computes the guest-visible result itself */
}

void pe_plat_bios_exit_critical(void)
{
    if (lock_depth > 0) lock_depth--;
}

void pe_plat_bios_flush_cache(void)
{
    /* no host state: native code has no instruction cache */
}

int PE_Irq_LockDepth(void)
{
    return lock_depth;
}

/* ── source-0 BIOS diagnostics snapshot ─────────────────────────────── */

#define SRC0_SLOT_ADDR      0x800945E8u   /* CPU interrupt source 0 handler slot */
#define SRC0_REGISTERED_ADDR 0x80094614u  /* registered-source mask halfword */
#define VBLANK_COUNTER      3u

static PeIrqSource0BiosState src0;
static uint64_t src0_order;

static void src0_observe(uint16_t *mask, pe_addr_t *slot, uint16_t *registered)
{
    *mask = PE_IRQ_GetMask();
    *slot = PE_LoadU32(SRC0_SLOT_ADDR);
    *registered = PE_LoadU16(SRC0_REGISTERED_ADDR);
}

void pe_plat_bios_change_clear_pad(uint32_t flag)
{
    src0.pad_calls++;
    src0.pad_argument = flag;
    src0.pad_clear_mode = flag;
    src0_observe(&src0.mask_at_pad_call, &src0.source0_slot_at_pad_call,
                 &src0.registered_mask_at_pad_call);
    src0.pad_call_order = ++src0_order;
}

uint32_t pe_plat_bios_change_clear_rcnt(uint32_t counter, uint32_t flag)
{
    if (counter != VBLANK_COUNTER) return 0;   /* only the VBlank counter belongs to source 0 */
    src0.vblank_calls++;
    src0.vblank_counter = counter;
    src0.vblank_argument = flag;
    src0.vblank_clear_mode = flag;
    src0_observe(&src0.mask_at_vblank_call, &src0.source0_slot_at_vblank_call,
                 &src0.registered_mask_at_vblank_call);
    src0.vblank_call_order = ++src0_order;
    return 0;
}

void PE_Irq_GetSource0BiosState(PeIrqSource0BiosState *out)
{
    if (out) *out = src0;
}

/* ── BIOS event table ───────────────────────────────────────────────── */

#define EVT_MAX          128
#define EVT_MODE_INTR    0x1000u   /* run the handler */
#define EVT_MODE_NOINTR  0x2000u   /* mark ready */
#define EVT_CLASS_SPU    0xF0000009u   /* SPU DMA completion, spec 0x20 */
#define EVT_SPEC_SPU     0x20u
#define EVT_CLASS_AUDIO  0xF2000002u   /* root counter 2 audio timer, spec 2, handler 8008E23C */
#define EVT_SPEC_AUDIO   2u
#define EVT_HANDLER_AUDIO 0x8008E23Cu
#define EVT_CLASS_CARD_HW 0xF0000011u  /* card events: delivered through the card bridge */
#define EVT_CLASS_CARD_SW 0xF4000001u
#define AUDIO_PRODUCER_BUSY_ADDR 0x8009D268u   /* nonzero while the command producer fills the FIFO */

typedef enum { EVT_FREE = 0, EVT_DISABLED, EVT_BUSY, EVT_READY } EvtStatus;

typedef struct {
    int handle;
    uint32_t cls, spec, mode;
    pe_addr_t handler;
    EvtStatus status;
} Evt;

static Evt evt[EVT_MAX];
/* Handles are opaque positive integers, never reused until reset.  The first one is 0x100: the SPU mode test
 * (DAY2_spu_mode) seeds the guest's SPU-event handle word with 0x100 and the DMA waits must find that event. */
#define EVT_FIRST_HANDLE 0x100
static int next_handle = EVT_FIRST_HANDLE;

static Evt *evt_find(int handle)
{
    int i;
    if (handle <= 0) return NULL;
    for (i = 0; i < EVT_MAX; i++)
        if (evt[i].status != EVT_FREE && evt[i].handle == handle) return &evt[i];
    return NULL;
}

int PE_Event_Open(uint32_t cls, uint32_t spec, uint32_t mode, pe_addr_t handler)
{
    int i;
    for (i = 0; i < EVT_MAX; i++) {
        if (evt[i].status == EVT_FREE) {
            evt[i].handle = next_handle++;
            evt[i].cls = cls;
            evt[i].spec = spec;
            evt[i].mode = mode;
            evt[i].handler = handler;
            evt[i].status = EVT_DISABLED;   /* OpenEvent: disabled until EnableEvent */
            return evt[i].handle;
        }
    }
    return -1;
}

int PE_Event_Enable(int handle)
{
    Evt *e = evt_find(handle);
    if (e) e->status = EVT_BUSY;
    return 1;
}

int PE_Event_Disable(int handle)
{
    Evt *e = evt_find(handle);
    if (e) e->status = EVT_DISABLED;
    return 1;
}

int PE_Event_Close(int handle)
{
    Evt *e = evt_find(handle);
    if (e) memset(e, 0, sizeof *e);
    return 1;
}

int PE_Event_Test(int handle)
{
    Evt *e = evt_find(handle);
    if (e && e->status == EVT_READY) {
        e->status = EVT_BUSY;
        return 1;
    }
    return 0;
}

int PE_Event_UnDeliver(uint32_t cls, uint32_t spec)
{
    int i;
    for (i = 0; i < EVT_MAX; i++)
        if (evt[i].status == EVT_READY && evt[i].mode == EVT_MODE_NOINTR &&
            evt[i].cls == cls && evt[i].spec == spec)
            evt[i].status = EVT_BUSY;
    return 1;
}

/* Deliver to every enabled-busy event of (cls, spec): mode 2000h events become ready; the handler of the
 * first mode-1000h event is returned for the caller to run (0 when there is none). */
static pe_addr_t evt_deliver(uint32_t cls, uint32_t spec, int *delivered)
{
    pe_addr_t run = 0;
    int i;
    *delivered = 0;
    for (i = 0; i < EVT_MAX; i++) {
        Evt *e = &evt[i];
        if (e->status != EVT_BUSY || e->cls != cls || e->spec != spec) continue;
        if (e->mode == EVT_MODE_NOINTR) {
            e->status = EVT_READY;
            (*delivered)++;
        } else if (e->mode == EVT_MODE_INTR && e->handler != 0u) {
            if (run == 0u) run = e->handler;
            (*delivered)++;
        }
    }
    return run;
}

pe_addr_t PE_Event_DeliverCard(uint32_t cls, uint32_t spec)
{
    int n;
    return evt_deliver(cls, spec, &n);
}

void PE_Event_Deliver(uint32_t cls, uint32_t spec)
{
    int n;
    if (cls == EVT_CLASS_CARD_HW || cls == EVT_CLASS_CARD_SW) {
        PE_BiosCard_Deliver(cls, spec);   /* runs the memory-card handler or marks the event ready */
        return;
    }
    (void)evt_deliver(cls, spec, &n);
}

int PE_Event_SpuDmaEnabled(void)
{
    int i;
    for (i = 0; i < EVT_MAX; i++)
        if ((evt[i].status == EVT_BUSY || evt[i].status == EVT_READY) &&
            evt[i].cls == EVT_CLASS_SPU && evt[i].spec == EVT_SPEC_SPU)
            return 1;
    return 0;
}

int PE_Event_DeliverSpuDma(void)
{
    int n;
    (void)evt_deliver(EVT_CLASS_SPU, EVT_SPEC_SPU, &n);
    return n > 0;
}

int PE_Event_ConsumeSpuDma(int handle)
{
    Evt *e = evt_find(handle);
    if (e && e->cls == EVT_CLASS_SPU && e->spec == EVT_SPEC_SPU && e->status == EVT_READY) {
        e->status = EVT_BUSY;
        return 1;
    }
    return 0;
}

int PE_Event_AudioTimerArmed(void)
{
    int i;
    if (lock_depth > 0) return 0;
    for (i = 0; i < EVT_MAX; i++)
        if ((evt[i].status == EVT_BUSY || evt[i].status == EVT_READY) &&
            evt[i].cls == EVT_CLASS_AUDIO && evt[i].spec == EVT_SPEC_AUDIO &&
            evt[i].handler == EVT_HANDLER_AUDIO)
            return 1;
    return 0;
}

void PE_Event_ServiceAudioCommands(void)
{
    if (PE_Port_ShouldStop()) return;
    if (!PE_Event_AudioTimerArmed()) return;
    if (PE_LoadU32(AUDIO_PRODUCER_BUSY_ADDR) != 0u) return;   /* the producer owns the FIFO */
    if (PE_AudioDriver_Active()) return;   /* the driver clock runs the whole tick, command queue included */
    func_8008CA84();
}

/* ── host / test reset owner ────────────────────────────────────────── */

#define CPU_IRQ_GUARD       0x800945E4u   /* halfword */
#define CPU_IRQ_DISPATCHING 0x800945E6u   /* halfword */
#define CPU_IRQ_SLOTS       0x800945E8u   /* eleven handler words */
#define CPU_IRQ_SLOT_COUNT  11u
#define CPU_IRQ_REGISTERED  0x80094614u   /* halfword */
#define CPU_IRQ_WATCHDOG    0x8009567Cu   /* word */
#define SPU_INIT_STATE_ADDR 0x8009B3ECu   /* SPU event-install state word (0 = not installed; B48A reset test) */
#define DMA_BUSY_ADDR       0x8009D24Cu
#define DMA_CALLBACK_ADDR   0x8009B434u
#define DMA_IRQ_HANDLE_ADDR 0x800956D0u

void PE_Sdk_ResetState(void)
{
    uint32_t i;
    memset(&g_pe_gte, 0, sizeof g_pe_gte);
    lock_depth = 0;
    memset(evt, 0, sizeof evt);
    next_handle = EVT_FIRST_HANDLE;
    memset(&src0, 0, sizeof src0);
    src0_order = 0;
    pe_plat_cdxetc_reset_state();
    PE_IRQ_Reset();
    PE_IRQ_DeliveryTraceReset();
    PE_MMIO_Reset();
    PE_Timer1_Reset();
    PE_Rcnt2_Reset();
    PE_Sio0_Reset();
    PE_CdReg_Reset();
    PE_GPU_Init();
    PE_MDEC_Init();
    PE_SpuDma_Reset();
    PE_Overlay_ResetResidency();
    /* Guest words the host resources above own: the CPU interrupt registration block and the SPU DMA
     * busy / callback / IRQ handle words.  Neighbouring guest state is left alone. */
    PE_StoreU16(CPU_IRQ_GUARD, 0u);
    PE_StoreU16(CPU_IRQ_DISPATCHING, 0u);
    for (i = 0; i < CPU_IRQ_SLOT_COUNT; i++) PE_StoreU32(CPU_IRQ_SLOTS + 4u * i, 0u);
    PE_StoreU16(CPU_IRQ_REGISTERED, 0u);
    PE_StoreU32(CPU_IRQ_WATCHDOG, 0u);
    PE_StoreU32(SPU_INIT_STATE_ADDR, 0u);
    PE_StoreU32(DMA_BUSY_ADDR, 0u);
    PE_StoreU32(DMA_CALLBACK_ADDR, 0u);
    PE_StoreU32(DMA_IRQ_HANDLE_ADDR, 0u);
}
