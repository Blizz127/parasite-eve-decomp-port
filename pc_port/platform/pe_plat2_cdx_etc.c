/* libetc veneers and VSync-family leaves (family cdx-etc, batch cdgen43): FlushCache, ChangeClearPAD,
 * ChangeClearRCnt, Enter/ExitCriticalSection, SetIntrMask, VSyncCallback, VSyncCallbacks and the
 * table-slot writer they share.
 *
 * Written clean-room from the black-box vector sets cdgen43 (docs/evidence/native-cdx-20261126), the
 * recorded seam contracts and psx-spx; no SDK source, decompiled source, tables or disassembly were
 * read.
 *
 * Behaviour (derivation in docs/evidence/native-cdx-20261126/cdx-etc/AUTHOR-REPORT.md):
 *  - EnterCriticalSection (SYSCALL 01h): psx-spx "Disables interrupts by clearing SR bit 2 and 10 ...
 *    Returns 1 if both bits were set, returns 0 if one or both of the bits were already zero".
 *  - ExitCriticalSection (SYSCALL 02h): "Enables interrupts by set SR bit 2 and 10 ... no return value".
 *  - ChangeClearRCnt C(0Ah): "returns the old (previous) flag value", flag kept per counter t=0..3.
 *  - ChangeClearPAD B(5Bh): remembers the pad/card auto-acknowledge policy; no return value.
 *  - FlushCache A(44h): flushes the I-cache; native host code has none, so only a count is kept.
 *  - SetIntrMask: exchanges the 16-bit I_MASK (PE_IRQ_ExchangeMask), returns the previous mask.
 *  - VSyncCallback(h) = table write with slot 4; VSyncCallbacks(n, h) and the table writer: read the
 *    previous handler at table + 4*n, store the new one only when it differs, return the previous one
 *    (PE_Callback_SetSlot, the seam contract).
 *
 * HOST SIDE.  The guest-visible results above are produced by this file alone.  The port additionally keeps
 * host bookkeeping that other host code gates on (critical-section depth that defers pending IRQ / audio
 * command delivery, the ChangeClear* snapshot used by the IRQ-registration diagnostics, the code-cache flush
 * hook).  That bookkeeping lives behind the pe_plat_bios_* host seams, now clean-room host contracts in
 * platform/pe_plat2_cdx_etc_seams.c (pe_plat/cdx_etc_seams.h); the five BIOS veneers forward to them and
 * ignore their results; the checkers stand those seams in as inert.  The raw names are kept on purpose: the
 * transition-loader test wraps them at link time (-Wl,--wrap). */
#include <stdint.h>
#define PE_CDXETC_HOST_SEAMS 1   /* keep the raw pe_plat_bios_* host-seam names in this TU */
#include "pe_plat/cdx_etc.h"
#include "pe_plat/bios_sys.h"   /* host bookkeeping seams pe_plat_bios_* (see HOST SIDE below) */
#include "pe_plat/irq_hw.h"
#include "pe_callback.h"

/* SR interrupt bits touched by the critical-section calls (psx-spx: bit 2 and bit 10). */
#define CDXETC_SR_IRQ_BITS ((1u << 2) | (1u << 10))
#define CDXETC_RCNT_COUNT 4u
#define CDXETC_VSYNC_SLOT 4u

static uint32_t sr_bits = CDXETC_SR_IRQ_BITS;
static uint32_t rcnt_policy[CDXETC_RCNT_COUNT];
static uint32_t pad_policy;
static uint32_t flush_calls;

void pe_plat_cdxetc_reset_state(void)
{
    uint32_t t;
    sr_bits = CDXETC_SR_IRQ_BITS;
    for (t = 0; t < CDXETC_RCNT_COUNT; t++) rcnt_policy[t] = 0;
    pad_policy = 0;
    flush_calls = 0;
}

int pe_plat_cdxetc_enter_critical(void)
{
    int both = (sr_bits & CDXETC_SR_IRQ_BITS) == CDXETC_SR_IRQ_BITS;
    sr_bits &= ~CDXETC_SR_IRQ_BITS;
    (void)pe_plat_bios_enter_critical();   /* host IRQ/audio gate depth */
    return both;
}

void pe_plat_cdxetc_exit_critical(void)
{
    sr_bits |= CDXETC_SR_IRQ_BITS;
    pe_plat_bios_exit_critical();          /* host IRQ/audio gate depth, pending delivery on unlock */
}

void pe_plat_cdxetc_flush_cache(void)
{
    flush_calls++;   /* no instruction cache in native host code */
    pe_plat_bios_flush_cache();
}

void pe_plat_cdxetc_change_clear_pad(uint32_t flag)
{
    pad_policy = flag;
    pe_plat_bios_change_clear_pad(flag);   /* host diagnostics snapshot */
}

uint32_t pe_plat_cdxetc_change_clear_rcnt(uint32_t counter, uint32_t flag)
{
    uint32_t old;
    /* psx-spx defines t = 0..2 (timers) and 3 (vblank); other indices are BIOS-undefined: no effect. */
    if (counter >= CDXETC_RCNT_COUNT) return 0;
    old = rcnt_policy[counter];
    rcnt_policy[counter] = flag;
    (void)pe_plat_bios_change_clear_rcnt(counter, flag);   /* host diagnostics snapshot */
    return old;
}

uint16_t pe_plat_cdxetc_set_intr_mask(uint16_t new_mask)
{
    return PE_IRQ_ExchangeMask(new_mask);
}

uint32_t pe_plat_cdxetc_vsync_callbacks(uint32_t slot, uint32_t handler)
{
    return PE_Callback_SetSlot(slot, handler);
}

uint32_t pe_plat_cdxetc_vsync_table_write(uint32_t slot, uint32_t handler)
{
    return PE_Callback_SetSlot(slot, handler);
}

uint32_t pe_plat_cdxetc_vsync_callback(uint32_t handler)
{
    return PE_Callback_SetSlot(CDXETC_VSYNC_SLOT, handler);
}

int pe_plat_cdxetc_interrupts_enabled(void)
{
    return (sr_bits & CDXETC_SR_IRQ_BITS) == CDXETC_SR_IRQ_BITS;
}

uint32_t pe_plat_cdxetc_rcnt_policy(uint32_t counter)
{
    return counter < CDXETC_RCNT_COUNT ? rcnt_policy[counter] : 0;
}

uint32_t pe_plat_cdxetc_pad_policy(void) { return pad_policy; }
uint32_t pe_plat_cdxetc_flush_count(void) { return flush_calls; }
