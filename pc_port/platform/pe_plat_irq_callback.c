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
 * Bound in pe_plat/irq_callback.h. */
#include "pe_plat/irq_callback.h"
#include "pe_plat/bios_sys.h"
#include "pe_guest_ram.h"
#include "pe_mmio.h"

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
#define ST_CLEAR4_LO  0x800956C0u  /* span 4: 8 words zeroed */
#define ST_CLEAR4_HI  0x800956DCu  /* last word of span 4 */
#define ST_WORD_A     0x80095650u  /* holds 0x800746A0 after reset */
#define ST_WORD_B     0x80095660u  /* holds 0x80074478 after reset */

#define H_VBLANK_DEFAULT 0x8007440Cu  /* guest code address in slot 0 after reset */
#define H_DMA_DEFAULT    0x80074520u  /* guest code address in slot 3 after reset */
#define V_WORD_A         0x800746A0u  /* value stored at ST_WORD_A (role unknown) */
#define V_WORD_B         0x80074478u  /* value stored at ST_WORD_B (role unknown) */

#define SRC_VBLANK 0u   /* I_STAT bit 0: also owns the pad clear and root counter 3 */
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
    PE_MMIO_Store(IO_T1_MODE, 4, T1_MODE_HBLANK_SYNC3);
    zero_span(ST_CLEAR2_LO, ST_CLEAR2_HI);
    PE_StoreU32(ST_WORD_A, V_WORD_A);
    PE_StoreU32(ST_WORD_B, V_WORD_B);
    PE_StoreU32(ST_CLEAR3_HI, 0u);       /* span 3 is cleared last word first */
    zero_span(ST_CLEAR3_LO, ST_CLEAR3_HI - 4u);
    (void)pe_plat_interrupt_callback(SRC_VBLANK, H_VBLANK_DEFAULT);
    zero_span(ST_CLEAR4_LO, ST_CLEAR4_HI);
    PE_MMIO_Store(IO_DICR, 4, 0u);
    (void)pe_plat_interrupt_callback(3u, H_DMA_DEFAULT);
    pe_plat_bios_exit_critical();
    return ST_READY;
}
