/*
 * Phase 6E-B6 — Guest-backed VBlank callback slot model.
 *
 * Replaces the Phase 6D-S single-callback host registry with the retail
 * callback-queue state machine, derived instruction-by-instruction:
 *
 *   func_80073D24(handler)            (asm/disc1/5F3E4.s @ 0x80073D24)
 *     → jump table D_8009564C field 0x14, forced slot a0 = 4
 *   field 0x14 installed by ResetCallback (func_80073E28 via func_80073C94)
 *     → return value of func_800743B4 = func_80074478
 *   func_80074478(slot, handler)      (asm/disc1/645F8.s @ 0x80074478)
 *     addr = D_8009568C + (slot << 2); prev = *addr;
 *     if (handler != prev) *addr = handler; return prev;
 *   func_8007440C                     (dispatcher, registered on event 0)
 *     D_800956AC++; for i in 0..7: if (slot[i]) slot[i]();  (no args)
 *
 * All retail-visible state lives in guest RAM at its retail addresses.
 * The host side only keeps a typed guest-address → host-function binding
 * map (full pointer width, never written to guest memory).
 */
#ifndef PE_CALLBACK_H
#define PE_CALLBACK_H

#include "pe_guest_ram.h"   /* pe_addr_t */

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*PECallback)(void);

/* Retail guest addresses (rodata/data, zero-initialized in the image) */
#define PE_CALLBACK_TABLE_ADDR    0x8009568Cu   /* D_8009568C: 8 handler slots */
#define PE_CALLBACK_COUNTER_ADDR  0x800956ACu   /* D_800956AC: dispatch count  */
#define PE_CALLBACK_SLOTS         8u

/* ── Lifecycle ──────────────────────────────────────────────────────── */

void PE_Callback_Init(void);        /* clear host bindings + diagnostics */

/* ── Host plumbing ──────────────────────────────────────────────────── */

/* Bind a guest function address to its host implementation.
 * Idempotent for identical (guest, host) pairs.  Returns 0 on success,
 * -1 on visible error (conflicting rebind, NULL host, zero guest, or a
 * full binding table). */
int PE_Callback_Bind(pe_addr_t guest, PECallback host);

/* ── Retail operations ──────────────────────────────────────────────── */

/* func_80074478 semantics: read previous handler at table + (slot << 2),
 * store the new one if different, return the previous value.  The slot
 * arithmetic is exact 32-bit retail arithmetic; out-of-guest addresses
 * hit the guest-RAM bounds policy. */
uint32_t  PE_Callback_SetSlot(uint32_t slot, pe_addr_t handler);

/* Raw read of table + (slot << 2). */
pe_addr_t PE_Callback_GetSlot(uint32_t slot);

/* func_800743B4 table-init portion: zero the dispatch counter first, then
 * zero all 8 slots (performed by ResetCallback's guard-passing call). */
void      PE_Callback_ResetTable(void);

/* func_8007440C semantics: increment the dispatch counter, then invoke
 * every non-NULL slot 0..7 in order with no arguments.  A slot holding
 * a guest address with no host binding is a visible error (logged and
 * counted), never a silent skip. */
void      PE_Callback_Dispatch(void);
/* Return zero when a callback cannot return; later slots remain untouched. */
int       PE_Callback_DispatchChecked(void);

/* ── Diagnostics ────────────────────────────────────────────────────── */

int PE_Callback_RegistrationCount(void);  /* non-null installs via SetSlot */
int PE_Callback_ErrorCount(void);         /* bind/dispatch visible errors  */

/* Value-only mutation-order evidence for the last ResetTable call. */
typedef struct {
    uint64_t counter_clear_order;
    uint64_t first_slot_clear_order;
    uint64_t last_slot_clear_order;
} PeCallbackResetTrace;

void PE_Callback_GetResetTrace(PeCallbackResetTrace *out);

#ifdef __cplusplus
}
#endif

#endif /* PE_CALLBACK_H */
