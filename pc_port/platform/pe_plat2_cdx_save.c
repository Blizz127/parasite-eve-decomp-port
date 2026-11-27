/* libpad bring-up pair, batch cdgen45: communication start 0x80082534 and
 * pad record initialiser 0x800844E4.
 *
 * Provenance: written clean-room from the black-box vector sets cdgen45
 * (docs/evidence/native-cdx-20261126), the recorded seam contracts and
 * psx-spx; no SDK source, decompiled source, tables or disassembly were
 * read.  The tail of 0x80082534 that the cdgen45 vectors cannot reach (an
 * installed card hook slot) follows the observed contract recorded in
 * platform/pe_plat_pad_gen34.c (cdgen34 vectors of 0x80082CF0).
 *
 * Which description the behaviour supports: both entries are libpad.  The
 * first clears the "communication running" word, enters the critical
 * section, acknowledges I_STAT bit 0 (VBlank), sets I_MASK bit 0, clears the
 * RCnt policy words and leaves the critical section: a PadStartCom wrapper,
 * not save-manager logic.  The second zeroes two 0xF0-byte pad records,
 * caches the two caller buffers in them (0xFF, 0x00 header), fills the
 * 0xFF name sentinels, installs the libpad hook table and the two libcard
 * callbacks and sets the running word: PadInitDirect.  Neither touches a
 * memory-card file or save data. */
#include <stdint.h>
#include "pe_sdk.h"
#include "pe_guest_ram.h"
#include "pe_plat/bios_sys.h"
#include "pe_plat/bios_intrp.h"
#include "pe_plat/cdx_save.h"

#define IO_I_STAT       0x1F801070u   /* write-0-to-acknowledge, bit n = source n */
#define IO_I_MASK       0x1F801074u   /* enable bit per source */
#define IRQ_VBLANK_BIT  1u

#define W_RUNNING       0x8009B75Cu   /* "communication running" word */
#define W_ARG           0x8009B758u   /* record array base, argument of the slot hook */
#define W_SLOT          0x8009B728u   /* libpad hook slot called by the start */
#define W_CNT_A         0x800A5AC4u
#define W_CNT_B         0x800A5AC0u
#define RECORD_STRIDE   0xF0u
#define QUEUE_BLOCK     0x800A5AB0u   /* interrupt-chain record, priority 2 */

void pe_plat_cdxsave_pad_comm_start(void)
{
    uint32_t arg, slot;
    PE_StoreU32(W_RUNNING, 0u);
    (void)pe_plat_bios_enter_critical();
    /* Kernel interrupt-chain services, in the order pe_plat_pad_gen34.c
     * (cdgen34, 0x80082CF0) issues them: dequeue C(03h) then enqueue C(02h)
     * of the record at 0x800A5AB0 on priority 2. */
    (void)pe_plat_bios_sys_deq_int_rp(2u, QUEUE_BLOCK);
    (void)pe_plat_bios_sys_enq_int_rp(2u, QUEUE_BLOCK);
    PE_StoreU32(IO_I_STAT, ~IRQ_VBLANK_BIT);
    PE_StoreU32(IO_I_MASK, PE_LoadU32(IO_I_MASK) | IRQ_VBLANK_BIT);
    (void)pe_plat_bios_change_clear_rcnt(3u, 0u);
    pe_plat_bios_exit_critical();
    /* Empty slot: the recorded vector path ends here (running stays 0). */
    arg = PE_LoadU32(W_ARG);
    slot = PE_LoadU32(W_SLOT);
    if (slot == 0u) return;
    /* Installed slot (the boot path: 0x800844E4 stores the record-reset hook
     * first): the hook runs for both records, re-reading both words before
     * each call, then the counters are cleared and the running word set
     * (contract of cdgen34, 0x80082CF0). */
    (void)PE_Card_ICall("D_8009B728 (card record reset)", slot, 1, arg, 0, 0);
    arg = PE_LoadU32(W_ARG);
    slot = PE_LoadU32(W_SLOT);
    if (slot == 0u) return;
    (void)PE_Card_ICall("D_8009B728 (card record reset)", slot, 1, arg + RECORD_STRIDE, 0, 0);
    PE_StoreU32(W_CNT_A, 0u);
    PE_StoreU32(W_CNT_B, 0u);
    PE_StoreU32(W_RUNNING, 1u);
}

#define REC0 0x800A5B70u
#define REC1 (REC0 + RECORD_STRIDE)
#define REC_ZERO_WORDS  120u          /* both records: 2 * 0xF0 bytes */

/* One pad record: +0x0C cleared, +0x10 self pointer, header bytes of the
 * caller buffer cached at +0x30 (read back before each byte store), two
 * pointers into the transfer areas at +0x3C/+0x40, 0xFF name sentinels at
 * +0x5D..+0x62. */
static void pad_record_setup(uint32_t rec, uint32_t area_a, uint32_t area_b)
{
    unsigned i;
    PE_StoreU32(rec + 0x0Cu, 0u);
    PE_StoreU32(rec + 0x10u, rec);
    PE_StoreU8(PE_LoadU32(rec + 0x30u), 0xFFu);
    PE_StoreU8(PE_LoadU32(rec + 0x30u) + 1u, 0u);
    PE_StoreU32(rec + 0x3Cu, area_a);
    PE_StoreU32(rec + 0x40u, area_b);
    for (i = 0; i < 6u; i++) PE_StoreU8(rec + 0x5Du + i, 0xFFu);
}

void pe_plat_cdxsave_pad_init_direct(uint32_t first, uint32_t second)
{
    unsigned i;
    PE_StoreU32(W_RUNNING, 0u);
    PE_StoreU32(0x8009B770u, 0u);
    /* libpad hook table */
    PE_StoreU32(0x8009B73Cu, 0x80084B78u);
    PE_StoreU32(0x8009B740u, 0x80084F8Cu);
    PE_StoreU32(0x8009B744u, 0x80084C4Cu);
    PE_StoreU32(0x8009B724u, 0x800846ACu);
    PE_StoreU32(W_SLOT,      0x80084644u);
    PE_StoreU32(0x8009B72Cu, 0x800847B0u);
    PE_StoreU32(0x8009B730u, 0x8008486Cu);
    PE_StoreU32(0x8009B734u, 0x80084AE8u);
    PE_StoreU32(0x8009B738u, 0x80084B20u);
    PE_StoreU32(W_ARG,       REC0);
    PE_StoreU32(0x8009B748u, 0x800847A0u);
    /* clear both records (recorded as 120 word stores from the first) */
    for (i = 0; i < REC_ZERO_WORDS; i++) PE_StoreU32(REC0 + 4u * i, 0u);
    PE_StoreU32(REC0 + 0x30u, first);
    PE_StoreU32(REC1 + 0x30u, second);
    pad_record_setup(REC0, 0x800A5AE0u, 0x800A5B28u);
    pad_record_setup(REC1, 0x800A5B03u, 0x800A5B4Bu);
    /* libcard callbacks and the interrupt-queue block */
    PE_StoreU32(0x800A5AB4u, 0x80082B70u);
    PE_StoreU32(0x800A5AB8u, 0x80082B08u);
    PE_StoreU32(0x800A5AB0u, 0u);
    PE_StoreU32(0x800A5ABCu, 0u);
    PE_StoreU32(W_RUNNING, 1u);
}
