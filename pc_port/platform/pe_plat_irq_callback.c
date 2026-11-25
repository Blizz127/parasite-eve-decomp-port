/* Interrupt-callback registry written clean-room from black-box vectors of
 * the original code (pc_port/tools/pe_irq_callback_vectors.py, checked by
 * pe_irq_callback_vectors_check.c) and from the hardware documentation, not
 * from SDK source or this port's earlier translation; see
 * docs/evidence/native-irq-callback-20261030/.
 *
 * Register meanings (psx-spx "Interrupts", "DMA Channels", "Timers"):
 *   I_STAT 1F801070h  write-0-to-acknowledge, one bit per source;
 *   I_MASK 1F801074h  one enable bit per source (bit n = source n);
 *   DPCR   1F8010F0h  DMA priority/enable, one nibble per channel 0-6
 *                     (bits 0-2 priority, bit 3 enable); bits 28-30 are the
 *                     CPU's memory priority, bit 31 has no real effect;
 *   DICR   1F8010F4h  DMA interrupt control (0 = masks and flags all clear);
 *   1F801114h         timer 1 mode: bit0 sync enable, bits1-2 sync mode
 *                     (3 = wait for the first VBlank, then free-run),
 *                     bit8 clock source (1 = HBlank for timer 1).
 *
 * What the vectors show:
 *   - A table of twelve guest words starts at 0x800945E8.  Slot n (n =
 *     0..10) holds the handler registered for I_STAT/I_MASK bit n.  Slot 11
 *     (0x80094614) is not a handler: its low halfword is a shadow of the
 *     enabled-source mask (9 after reset = VBlank | DMA).  Every registration
 *     also read-modify-writes that halfword with the same set/clear, so source
 *     11 (one past the real sources) stores the handler word there and then
 *     leaves handler|0x800 in it.
 *   - Until reset has run (halfword at 0x800945E4 still zero) registration is
 *     inert: it returns the slot's current word and changes nothing.
 *   - Registering is: I_MASK <- 0, optional BIOS clears for the sources that
 *     own a pad or root-counter hook, I_MASK <- previous I_MASK with the
 *     source's bit set (handler != 0) or cleared (handler == 0).  Registering
 *     the handler that is already there does nothing at all.
 *   - Reset is: clear the state block, then run the mask/DMA/timer set-up and
 *     register the two built-in handlers (source 0 and source 3) through the
 *     same registration path.  It runs only while the halfword at 0x800945E4
 *     is zero and returns the address of that flag word; a second call is a
 *     no-op returning 0.
 *   - DMA completion callbacks: a second table of eight guest words starts at
 *     0x800956C0 (channel n at +4n, n = 0..7; reset zero-fills it).  Setting
 *     a channel's callback stores the word, then rewrites DICR (psx-spx
 *     "DMA Channels"): bits 0-23 of the current DICR are kept, the master
 *     enable bit 23 is set, and the channel's enable bit 16+n is set
 *     (handler != 0) or cleared (handler == 0), per the README's psx-spx DICR summary (bit 23 master enable, bits 16-22 channel enables, 24-30 flags).  The clear is applied after
 *     the master set, so channel 7 (whose enable bit is bit 23 itself) ends
 *     with the master bit clear.  The flag bits 24-30 are not written back
 *     as ones, so pending DMA flags survive.  Registering the handler that is
 *     already there does nothing at all; the previous word is returned.
 *     Nothing touches I_MASK, I_STAT or the source-3 slot: the source-3
 *     handler installed by reset is independent of the per-channel table.
 *   - Stop/Restart suspend and resume the whole system.  Stop parks the live
 *     I_MASK (halfword) at 0x80094616 and the live DPCR (word) at 0x80094618
 *     and silences the hardware; Restart puts them back.  The "ready" halfword
 *     at 0x800945E4 is the on/off switch: Stop acts only while it is non-zero
 *     and clears it, Restart acts only while it is zero and sets it, so a
 *     repeated or out-of-order call is a no-op returning 0.  Both return the
 *     address of the ready word when they act.  The registered handlers, the
 *     mask shadow at 0x80094614 and the DMA callback table are left alone, and
 *     while stopped interrupt registration is inert (it keys on the same
 *     ready halfword) but DMA callback registration still works.
 *   - VBlank callbacks: eight guest words at 0x8009568C (slot n at +4n) hold
 *     per-frame callbacks; the word after them (0x800956AC) is a frame
 *     counter.  The VBlank handler bumps the counter, then runs the non-zero
 *     slots in order.  Start-VSync re-initialises timer 1 and zeroes counter
 *     and slots.
 *   - DMA interrupt handler: ascending walks over channels 0..6 (DICR read
 *     once per walk, walks repeated from channel 0 until one finds nothing, so
 *     flags latched by callbacks are serviced in the same call); a channel
 *     whose DICR flag (bit 24+n) is set has the flag acknowledged (writing 1 to it, psx-spx "DMA
 *     Channels") and runs the channel's callback if one is registered.
 *     Start-DMA zeroes the callback table and DICR.
 *   - CPU interrupt dispatcher: serves the sources in I_STAT & I_MASK
 *     (bits 0..10) by ascending walks over a fresh snapshot, acknowledging each
 *     source before running its slot handler; see pe_plat_irq_dispatch.
 *     0x800945E6 (upper half of the ready word) brackets a dispatch, and
 *     0x8009567C counts consecutive dispatches that ended with an unserved source.
 * Bound in pe_plat/irq_callback.h. */
#include "pe_plat/irq_callback.h"
#include "pe_plat/bios_sys.h"
#include "pe_guest_ram.h"
#include "pe_mmio.h"
#include "pe_bios_string.h"

#define IO_I_STAT 0x1F801070u
#define IO_I_MASK 0x1F801074u
#define IO_DPCR   0x1F8010F0u
#define IO_DICR   0x1F8010F4u
#define IO_T1_MODE 0x1F801114u

#define DPCR_ALL_PRIO3_DISABLED 0x33333333u  /* nibbles 0-7: priority 3, enable bit clear */
#define T1_MODE_HBLANK_SYNC3    0x107u       /* sync on, mode 3 (VBlank once), HBlank clock */

/* Guest state block, all inside the original's zero-filled region. */
#define ST_READY      0x800945E4u  /* 1 once reset has run (halfword tested, word stored) */
#define ST_HANDLERS   0x800945E8u  /* slot n = handler for source n, n = 0..11 */
#define ST_MASK_SLOT  11u          /* slot 11 = enabled-source mask shadow */
#define ST_CLEAR1_LO  0x800945E4u  /* zero-filled span 1: ready flag, slots, one word */
#define ST_CLEAR1_HI  0x80094618u  /* last word of span 1 (written 0 in every vector) */
#define ST_CLEAR2_LO  0x8009464Cu  /* span 2 (the 12 words at 0x8009461C are a jump
                                    * buffer the original owns; never touched) */
#define ST_CLEAR2_HI  0x80095648u  /* last word of span 2 */
#define ST_CLEAR3_LO  0x8009568Cu  /* span 3: 9 words zeroed past the main block */
#define ST_CLEAR3_HI  0x800956ACu  /* last word of span 3 */
#define ST_CLEAR4_LO  0x800956C0u  /* span 4: the 8-word DMA callback table (ST_DMA_HANDLERS) */
#define ST_CLEAR4_HI  0x800956DCu  /* last word of span 4 */
#define ST_WORD_A     0x80095650u  /* holds 0x800746A0 after reset */
#define ST_WORD_B     0x80095660u  /* holds 0x80074478 after reset */

#define H_VBLANK_DEFAULT 0x8007440Cu  /* guest code address in slot 0 after reset */
#define H_DMA_DEFAULT    0x80074520u  /* guest code address in slot 3 after reset */
#define ST_DMA_HANDLERS 0x800956C0u /* slot n = completion callback for DMA channel n */

#define DICR_LOW_MASK     0x00FFFFFFu /* bits 0-23 read/write; 24-31 flags, not written back */
#define DICR_MASTER_ENABLE 0x00800000u /* bit 23 */
#define DICR_CHAN_ENABLE_SHIFT 16u     /* bits 16-22: per-channel IRQ enable */

#define ST_SAVED_IMASK   0x80094616u  /* halfword: I_MASK parked by Stop (upper half of slot 11) */
#define ST_SAVED_DPCR    0x80094618u  /* word: DPCR parked by Stop (last word of the span reset clears) */
#define DPCR_PRIO_BITS    0x77777777u /* bits 0-2 of each nibble; bit 3 (channel enable, bit 31 included) is dropped */

#define V_WORD_A         0x800746A0u  /* value stored at ST_WORD_A (role unknown) */
#define V_WORD_B         0x80074478u  /* value stored at ST_WORD_B (role unknown) */

#define ST_VSYNC_SLOTS   0x8009568Cu  /* eight VBlank callback words, slot n at +4n (== ST_CLEAR3_LO) */
#define VSYNC_SLOT_COUNT 8u
#define ST_VSYNC_COUNT   0x800956ACu  /* frame counter, incremented per VBlank (== ST_CLEAR3_HI) */
#define DMA_CHANNEL_COUNT 7u          /* channels 0..6 have flags and callbacks */
#define DICR_FLAG_SHIFT  24u          /* bits 24-30: per-channel IRQ flag, write 1 to reset */
#define DICR_FORCE_IRQ   0x00008000u  /* bit 15: feeds DICR bit 31 (README DICR summary: bit31 = bit15 OR ...) */
#define FMT_DMA_DICR     0x8001177Cu  /* guest format string: diagnostic, argument = DICR */
#define FMT_DMA_CHANNEL  0x80011798u  /* guest format string: one line per channel, argument = channel */

#define ST_IN_DISPATCH   0x800945E6u  /* halfword: 1 while a dispatch runs (upper half of the ready word) */
#define ST_STUCK_COUNT   0x8009567Cu  /* word: consecutive dispatches that ended with an unserved pending source */
#define STUCK_LIMIT      0x801        /* diagnostic once the incremented count exceeds this (vectors: 0x800 -> 0x801 silent, 0x801 -> report) */
#define IRQ_SOURCE_COUNT 11u          /* sources 0..10 have handler slots; slot 11 is the mask shadow */
#define FMT_IRQ_NOT_READY 0x80011740u /* guest format string: interrupt before reset, argument = raw I_STAT */
#define FMT_IRQ_STUCK     0x8001175Cu /* guest format string: stuck source, arguments = live I_STAT, live I_MASK */

#define SRC_VBLANK 0u   /* I_STAT bit 0: also owns the pad clear and root counter 3 */
#define SRC_DMA    3u   /* I_STAT bit 3 */
#define SRC_RCNT0  4u   /* bits 4..6: root counters 0..2 */
#define SRC_RCNT2  6u

static void zero_span(uint32_t lo, uint32_t hi)
{
    uint32_t a;
    for (a = lo; a <= hi; a += 4u) PE_StoreU32(a, 0u);
}

/* Sources 0..11 are covered by the vectors; the original has no bound, so a
 * larger source writes past the table as it would. */
uint32_t pe_plat_interrupt_callback(uint32_t source, uint32_t handler)
{
    uint32_t slot = ST_HANDLERS + source * 4u;
    uint32_t old = PE_LoadU32(slot);
    uint32_t bit = 1u << (source & 31u);
    uint32_t mask = PE_MMIO_Load(IO_I_MASK, 2);
    uint32_t shadow;

    if (PE_LoadU16(ST_READY) == 0u || old == handler) return old;
    PE_MMIO_Store(IO_I_MASK, 2, 0u);
    PE_StoreU32(slot, handler);
    shadow = PE_LoadU16(ST_HANDLERS + ST_MASK_SLOT * 4u);
    PE_StoreU16(ST_HANDLERS + ST_MASK_SLOT * 4u,
        (uint16_t)(handler ? shadow | bit : shadow & ~bit));
    if (source == SRC_VBLANK) {
        pe_plat_bios_change_clear_pad(handler == 0u);
        (void)pe_plat_bios_change_clear_rcnt(3u, handler == 0u);
    } else if (source >= SRC_RCNT0 && source <= SRC_RCNT2) {
        (void)pe_plat_bios_change_clear_rcnt(source - SRC_RCNT0, handler == 0u);
    }
    PE_MMIO_Store(IO_I_MASK, 2, handler ? mask | bit : mask & ~bit);
    return old;
}

uint32_t pe_plat_reset_callback(void)
{
    if (PE_LoadU16(ST_READY) != 0u) return 0u;

    PE_MMIO_Store(IO_I_MASK, 2, 0u);
    PE_MMIO_Store(IO_I_STAT, 2, 0u);
    PE_MMIO_Store(IO_DPCR, 4, DPCR_ALL_PRIO3_DISABLED);
    zero_span(ST_CLEAR1_LO, ST_CLEAR1_HI);
    PE_StoreU16(ST_READY, 1u);
    zero_span(ST_CLEAR2_LO, ST_CLEAR2_HI);
    PE_StoreU32(ST_WORD_A, V_WORD_A);
    PE_StoreU32(ST_WORD_B, V_WORD_B);
    (void)pe_plat_start_vsync();
    (void)pe_plat_start_dma();
    pe_plat_bios_exit_critical();
    return ST_READY;
}

/* The original has no channel bound.  Channels 0..7 own the table; a larger
 * channel addresses the word at 0x800956C0 + 4*channel (whatever the guest
 * keeps there: 8 reads/writes 0x800956E0, 31 reads 0x8009573C) and sets or
 * clears bit (16+channel) mod 32 of DICR, so 8..10 reach the flag bits 24-26
 * (written as ones, i.e. acknowledging them) and 31 wraps to bit 15.
 * Unlike the interrupt registry this has no reset gate: the setter the
 * original reaches (0x800746A0) stores, writes DICR and returns the previous
 * word identically before and after ResetCallback, and the host binds the
 * DMACallback entry to the same behaviour. */
uint32_t pe_plat_dma_callback(uint32_t channel, uint32_t handler)
{
    uint32_t slot, old, bit, dicr;

    slot = ST_DMA_HANDLERS + channel * 4u;
    old = PE_LoadU32(slot);
    if (old == handler) return old;

    PE_StoreU32(slot, handler);
    bit = 1u << ((DICR_CHAN_ENABLE_SHIFT + channel) & 31u);
    dicr = (PE_MMIO_Load(IO_DICR, 4) & DICR_LOW_MASK) | DICR_MASTER_ENABLE;
    PE_MMIO_Store(IO_DICR, 4, handler ? dicr | bit : dicr & ~bit);
    return old;
}

/* Suspend: park I_MASK and DPCR, then mask every interrupt source, acknowledge
 * all of I_STAT and clear bit 3 of all eight DPCR nibbles (channel enables,
 * and bit 31); the vectors show 0xFFFFFFFF -> 0x77777777, 0xC4444444 ->
 * 0x44444444, so bits 0-2 of every nibble (priorities, 28-30) stay.
 * The critical section entered here is not left again; Restart's exit
 * pairs with it. */
uint32_t pe_plat_stop_callback(void)
{
    uint32_t dpcr;

    if (PE_LoadU16(ST_READY) == 0u) return 0u;

    (void)pe_plat_bios_enter_critical();
    dpcr = PE_MMIO_Load(IO_DPCR, 4);
    PE_StoreU16(ST_SAVED_IMASK, (uint16_t)PE_MMIO_Load(IO_I_MASK, 2));
    PE_StoreU32(ST_SAVED_DPCR, dpcr);
    PE_MMIO_Store(IO_I_MASK, 2, 0u);
    PE_MMIO_Store(IO_I_STAT, 2, 0u);
    PE_MMIO_Store(IO_DPCR, 4, dpcr & DPCR_PRIO_BITS);
    PE_StoreU16(ST_READY, 0u);
    return ST_READY;
}

/* Resume: mark the system ready and restore the parked I_MASK and DPCR.
 * I_STAT is not restored (acknowledged flags stay lost).  Without a prior Stop
 * or Reset the parked words are whatever guest RAM holds. */
uint32_t pe_plat_restart_callback(void)
{
    if (PE_LoadU16(ST_READY) != 0u) return 0u;

    PE_StoreU16(ST_READY, 1u);
    PE_MMIO_Store(IO_I_MASK, 2, PE_LoadU16(ST_SAVED_IMASK));
    PE_MMIO_Store(IO_DPCR, 4, PE_LoadU32(ST_SAVED_DPCR));
    pe_plat_bios_exit_critical();
    return ST_READY;
}

/* VBlank interrupt handler: bump the frame counter (also before reset; it
 * wraps), then run the non-zero callback slots in order.  The counter store
 * comes first.  A host stop from a callback ends the handler at once. */
uint32_t pe_plat_vsync_handler(void)
{
    uint32_t n, fn;

    PE_StoreU32(ST_VSYNC_COUNT, PE_LoadU32(ST_VSYNC_COUNT) + 1u);
    for (n = 0; n < VSYNC_SLOT_COUNT; n++) {
        fn = PE_LoadU32(ST_VSYNC_SLOTS + n * 4u);
        if (fn != 0u && !pe_plat_invoke_guest_callback(fn)) return 0u;
    }
    return 0u;
}

/* DMA interrupt handler.  Walk channels 0..6 in ascending order, reading
 * DICR once at the start of each walk (callbacks may latch further flags,
 * but those are only seen by the next walk); repeat the walk from channel 0
 * until a walk finds no flag set.  Vectors: 4 and 6 pending, ch4 latches 2 ->
 * 4, 6, 2; 2 and 6 pending, ch2 latches 4 -> 2, 6, 4 (not 2, 4, 6).
 * For a channel flagged in the snapshot: acknowledge it by re-reading DICR
 * and writing its low 24 bits back with that flag bit as 1 (a callback that
 * cleared a later channel's enable bit keeps it cleared; psx-spx: flags are
 * write-1-to-reset, so the other flags are written 0 and survive), then run
 * the channel callback if registered.
 * The enable bits are not consulted (the vectors acknowledge and call with
 * the enable clear).  Afterwards DICR is read again; if the force bit (15) is
 * set a diagnostic with that DICR value and one line per channel are printed
 * through the BIOS. */
uint32_t pe_plat_dma_handler(void)
{
    uint32_t dicr, snap, pending;
    uint32_t ch, fn, arg[2];

    do {
        pending = 0u;
        snap = PE_MMIO_Load(IO_DICR, 4);
        for (ch = 0; ch < DMA_CHANNEL_COUNT; ch++) {
            if (!(snap & (1u << (DICR_FLAG_SHIFT + ch)))) continue;
            pending = 1u;
            dicr = PE_MMIO_Load(IO_DICR, 4);
            PE_MMIO_Store(IO_DICR, 4, (dicr & DICR_LOW_MASK) | (1u << (DICR_FLAG_SHIFT + ch)));
            fn = PE_LoadU32(ST_DMA_HANDLERS + ch * 4u);
            if (fn != 0u && !pe_plat_invoke_guest_callback(fn)) return 0u;
        }
    } while (pending);
    dicr = PE_MMIO_Load(IO_DICR, 4);
    if (dicr & DICR_FORCE_IRQ) {
        (void)PE_Bios_Printf(FMT_DMA_DICR, &dicr, 1);
        for (ch = 0; ch < DMA_CHANNEL_COUNT; ch++) {
            arg[0] = ch;
            arg[1] = pe_plat_read_dma_madr(ch);
            (void)PE_Bios_Printf(FMT_DMA_CHANNEL, arg, 2);
        }
    }
    return 0u;
}

/* Initialise the VBlank system: timer 1 mode as Reset sets it, zero the
 * frame counter and slots (counter first, slots ascending), then re-register
 * the default VBlank handler on source 0 through the normal registry path
 * (so it replaces any other source-0 handler and is inert before reset).
 * Returns V_WORD_B. */
uint32_t pe_plat_start_vsync(void)
{
    uint32_t n;

    PE_MMIO_Store(IO_T1_MODE, 4, T1_MODE_HBLANK_SYNC3);
    PE_StoreU32(ST_VSYNC_COUNT, 0u);
    for (n = 0; n < VSYNC_SLOT_COUNT; n++) PE_StoreU32(ST_VSYNC_SLOTS + n * 4u, 0u);
    (void)pe_plat_interrupt_callback(SRC_VBLANK, H_VBLANK_DEFAULT);
    return V_WORD_B;
}

/* Initialise the DMA system: zero the eight callback words ascending, clear
 * DICR (a plain 0 write), then re-register the default DMA handler on source 3
 * through the registry (replaces any other source-3 handler; inert before
 * reset).  Returns V_WORD_A. */
uint32_t pe_plat_start_dma(void)
{
    zero_span(ST_CLEAR4_LO, ST_CLEAR4_HI);
    PE_MMIO_Store(IO_DICR, 4, 0u);
    (void)pe_plat_interrupt_callback(SRC_DMA, H_DMA_DEFAULT);
    return V_WORD_A;
}

/* CPU interrupt dispatcher.  Before reset (ready halfword zero) it only prints
 * the raw I_STAT and returns.  Otherwise it brackets the work with the
 * 0x800945E6 halfword (1 ... 0) and repeats ascending walks over sources
 * 0..10.  A walk's pending set is a snapshot of
 *     I_STAT & I_MASK & (halfword at 0x80094614, the registry's mask shadow),
 * so a source raised by a handler is only served by the next walk (vectors:
 * sources 1 and 5 pending, the source-1 handler raises 2 -> 1, 5, 2; 4 and 6
 * pending, 4 raises 0 -> 4, 6, 0).  Every pending source is acknowledged (I_STAT
 * <- ~bit, psx-spx "Interrupts": write 0 to reset) and then, only if its slot
 * word is non-zero, its handler runs (vectors: shadow bit set, slot zeroed
 * by other code -> acknowledged, no call).  Handlers run through the host
 * callback service (which runs the two built-in defaults as the native VBlank
 * / DMA handlers).  The walks end when the pending set is empty.
 * Watchdog: if I_STAT & I_MASK still has a source bit that no walk could take
 * (e.g. enabled in I_MASK but absent from the shadow; it stays unacknowledged),
 * the stuck counter is incremented; once it passes the limit the live I_STAT
 * and I_MASK are printed, all of I_STAT is acknowledged and the counter
 * restarts at zero.  Otherwise (nothing left) the counter is stored as zero:
 * the vectors list a store to 0x8009567C in every post-reset dispatch.
 * The original leaves through ReturnFromException, which the host does not
 * have, so this returns 0. */
uint32_t pe_plat_irq_dispatch(void)
{
    uint32_t pending, left, src, fn, count, args[2];
    const uint32_t sources = (1u << IRQ_SOURCE_COUNT) - 1u;

    if (PE_LoadU16(ST_READY) == 0u) {
        pending = PE_MMIO_Load(IO_I_STAT, 2);
        (void)PE_Bios_Printf(FMT_IRQ_NOT_READY, &pending, 1);
        return 0u;
    }
    PE_StoreU16(ST_IN_DISPATCH, 1u);
    for (;;) {
        pending = PE_MMIO_Load(IO_I_STAT, 2) & PE_MMIO_Load(IO_I_MASK, 2) &
            PE_LoadU16(ST_HANDLERS + ST_MASK_SLOT * 4u) & sources;
        if (pending == 0u) break;
        for (src = 0; src < IRQ_SOURCE_COUNT; src++) {
            if (!(pending & (1u << src))) continue;
            PE_MMIO_Store(IO_I_STAT, 2, ~(1u << src) & 0xFFFFu);
            fn = PE_LoadU32(ST_HANDLERS + src * 4u);
            if (fn != 0u && !pe_plat_invoke_guest_callback(fn)) return 0u;
        }
    }
    left = PE_MMIO_Load(IO_I_STAT, 2) & PE_MMIO_Load(IO_I_MASK, 2) & sources;
    count = 0u;
    if (left != 0u) {
        count = PE_LoadU32(ST_STUCK_COUNT) + 1u;
        if ((int32_t)count > STUCK_LIMIT) {
            args[0] = PE_MMIO_Load(IO_I_STAT, 2);
            args[1] = PE_MMIO_Load(IO_I_MASK, 2);
            (void)PE_Bios_Printf(FMT_IRQ_STUCK, args, 2);
            PE_MMIO_Store(IO_I_STAT, 2, 0u);
            count = 0u;
        }
    }
    PE_StoreU32(ST_STUCK_COUNT, count);
    PE_StoreU16(ST_IN_DISPATCH, 0u);
    return 0u;
}
