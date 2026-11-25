/*
 * Phase 6E-B6 — Guest-backed VBlank callback slot model implementation.
 *
 * Retail state (guest RAM, exact retail addresses):
 *   0x8009568C..0x800956AB — D_8009568C, 8 handler slots (guest addresses)
 *   0x800956AC             — D_800956AC, dispatch counter
 *
 * Host state (never guest-visible):
 *   binding map guest function address → host PECallback, full width.
 */
#include "pe_callback.h"
#include "pe_plat/irq_callback.h"
#include "pe_irq_delivery.h"
#include "game_port.h"
#include "pe_guestcode.h"
#include "host_framebuffer.h"
#include <stdio.h>

#define PE_CALLBACK_MAX_BINDS 16

typedef struct {
    pe_addr_t  guest;
    PECallback host;
} PECallbackBind;

static PECallbackBind g_binds[PE_CALLBACK_MAX_BINDS];
static int            g_bind_count = 0;
static int            g_reg_count  = 0;
static int            g_err_count  = 0;
static PeCallbackResetTrace g_reset_trace;
static uint64_t       g_reset_order = 0;

void PE_Callback_Init(void)
{
    HostFB_SetVBlankDispatch(PE_Callback_DispatchChecked);
    g_bind_count = 0;
    g_reg_count  = 0;
    g_err_count  = 0;
    g_reset_trace.counter_clear_order = 0;
    g_reset_trace.first_slot_clear_order = 0;
    g_reset_trace.last_slot_clear_order = 0;
    g_reset_order = 0;
}

int PE_Callback_Bind(pe_addr_t guest, PECallback host)
{
    if (guest == 0 || host == NULL) {
        fprintf(stderr, "[CALLBACK] bind: invalid guest 0x%08X or host %p\n",
                guest, (void *)host);
        g_err_count++;
        return -1;
    }
    for (int i = 0; i < g_bind_count; i++) {
        if (g_binds[i].guest == guest) {
            if (g_binds[i].host == host) {
                return 0;               /* idempotent rebind */
            }
            fprintf(stderr, "[CALLBACK] bind: guest 0x%08X already bound\n",
                    guest);
            g_err_count++;
            return -1;
        }
    }
    if (g_bind_count >= PE_CALLBACK_MAX_BINDS) {
        fprintf(stderr, "[CALLBACK] bind: table full (%d entries)\n",
                PE_CALLBACK_MAX_BINDS);
        g_err_count++;
        return -1;
    }
    g_binds[g_bind_count].guest = guest;
    g_binds[g_bind_count].host  = host;
    g_bind_count++;
    return 0;
}

static PECallback Resolve(pe_addr_t guest)
{
    for (int i = 0; i < g_bind_count; i++) {
        if (g_binds[i].guest == guest) {
            return g_binds[i].host;
        }
    }
    return NULL;
}

uint32_t PE_Callback_SetSlot(uint32_t slot, pe_addr_t handler)
{
    /* func_80074478: sll a0,2; addu a0,a0,table; lw v0,(a0);
     * if (a1 != v0) sw a1,(a0);  return v0 (exact 32-bit wrap). */
    pe_addr_t addr = (pe_addr_t)(PE_CALLBACK_TABLE_ADDR + (slot << 2));
    uint32_t  prev = PE_LoadU32(addr);
    if (handler != prev) {
        PE_StoreU32(addr, handler);
        if (handler != 0) {
            g_reg_count++;
        }
    }
    return prev;
}

pe_addr_t PE_Callback_GetSlot(uint32_t slot)
{
    pe_addr_t addr = (pe_addr_t)(PE_CALLBACK_TABLE_ADDR + (slot << 2));
    return PE_LoadU32(addr);
}

void PE_Callback_ResetTable(void)
{
    /* func_800743B4 writes D_800956AC = 0 at 0x800743D8, then calls
     * func_800744A4(D_8009568C, 8) at 0x800743DC. */
    g_reset_order = 0;
    PE_StoreU32(PE_CALLBACK_COUNTER_ADDR, 0);
    g_reset_trace.counter_clear_order = ++g_reset_order;
    for (uint32_t i = 0; i < PE_CALLBACK_SLOTS; i++) {
        PE_StoreU32((pe_addr_t)(PE_CALLBACK_TABLE_ADDR + i * 4u), 0);
        if (i == 0u) {
            g_reset_trace.first_slot_clear_order = ++g_reset_order;
        } else {
            g_reset_order++;
        }
        if (i + 1u == PE_CALLBACK_SLOTS) {
            g_reset_trace.last_slot_clear_order = g_reset_order;
        }
    }
}

void PE_Callback_GetResetTrace(PeCallbackResetTrace *out)
{
    if (out != NULL) {
        *out = g_reset_trace;
    }
}

/* Host service for the clean-room interrupt handlers
 * (platform/pe_plat_irq_callback.c): call the guest callback at `guest` with
 * no arguments, as the retail jalr through a callback slot would.  A host
 * bind wins, then the verified guest-code registry; anything else is a loud
 * stop.  Returns 0 when the port has stopped (the caller returns at once). */
int pe_plat_invoke_guest_callback(uint32_t guest)
{
    uint32_t epoch = PE_Port_StopEpoch();
    int dma_ok;
    if (PE_IRQ_DmaCallbackService(guest, &dma_ok))   /* DMA-specific routing */
        return dma_ok && PE_Port_StopEpoch() == epoch;
    if (PE_IRQ_CpuCallbackService(guest, &dma_ok))   /* CPU handler routing */
        return dma_ok && PE_Port_StopEpoch() == epoch;
    PECallback fn = Resolve(guest);
    const PeGuestCodeEntry *code = fn ? NULL : PE_GuestCode_Resolve(guest);
    if (fn) {
        fn();
    } else if (code && code->thunk) {
        (void)code->thunk(0u, 0u, 0u, 0u);
    } else {
        fprintf(stderr,
                "[CALLBACK] dispatch: no host binding for guest 0x%08X\n",
                guest);
        g_err_count++;
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    return PE_Port_StopEpoch() == epoch;
}

int PE_Callback_DispatchChecked(void)
{
    /* The VBlank handler is the clean-room pe_plat_vsync_handler (guest
     * 0x8007440C); this host entry reports whether it ran without a stop. */
    uint32_t epoch = PE_Port_StopEpoch();
    (void)pe_plat_vsync_handler();
    return PE_Port_StopEpoch() == epoch;
}

void PE_Callback_Dispatch(void)
{ (void)PE_Callback_DispatchChecked(); }

int PE_Callback_RegistrationCount(void)
{
    return g_reg_count;
}

int PE_Callback_ErrorCount(void)
{
    return g_err_count;
}
