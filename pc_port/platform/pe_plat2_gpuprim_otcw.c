/* Slot-11 ordering-table clear worker 0x80076354, written clean-room from the
 * black-box vector set gpu2c (docs/evidence/native-gpu2-20261126), the port's
 * host GPU/DMA model API and psx-spx (DMA channel 6 / OTC: MADR, BCR, CHCR,
 * DPCR enable bit 27); no SDK source, decompiled source, tables or
 * disassembly were read.
 *
 * Derivation (traced to the vectors):
 *  - The hardware registers are reached through pointer words in RAM:
 *    DPCR 0x80095870, DMA6 CHCR 0x8009586C, MADR 0x80095864, BCR 0x80095868.
 *  - Sequence: DPCR |= 0x08000000; CHCR = 0; MADR = ot + 4 * (count - 1)
 *    (32-bit wrap); BCR = count; CHCR = 0x11000002; then the deadline seeder
 *    0x800773D0 is called once; then CHCR is re-read (pointer word re-read)
 *    until its busy bit 24 is clear, calling the timeout check 0x80077404 after
 *    every busy read: a non-zero result ends the wait and the function returns -1; an idle
 *    channel returns count.
 *  - Host-only additions (not observable in the vectors): the poll leaves when
 *    PE_Port_ShouldStop() is set and has a stuck-wait budget that requests a
 *    stop; register pointer words that are neither RAM nor a modelled register
 *    use the architectural register address instead. */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_plat/gpuprim.h"
#include "pe_plat/gpu_regs.h"
#include "pe_plat/gpu_timeout.h"

uint32_t pe_plat_gpu_seed_deadline_host(void);   /* guest 0x800773D0 */

#define P_MADR6 0x80095864u
#define P_BCR6  0x80095868u
#define P_CHCR6 0x8009586Cu
#define P_DPCR  0x80095870u
#define HW_MADR6 0x1F8010E0u
#define HW_BCR6  0x1F8010E4u
#define HW_CHCR6 0x1F8010E8u
#define HW_DPCR  0x1F8010F0u
#define DPCR_OTC_ENABLE 0x08000000u
#define CHCR_START_OTC  0x11000002u
#define CHCR_BUSY       0x01000000u
#define SPIN_BUDGET     4000000u

int pe_plat_gpuprim_otc_worker(uint32_t ot, int count)
{
    uint32_t dpcr = pe_gpu_reg_word(P_DPCR, HW_DPCR, 4);
    uint32_t polls = 0;
    PE_StoreU32(dpcr, PE_LoadU32(dpcr) | DPCR_OTC_ENABLE);
    PE_StoreU32(pe_gpu_reg_word(P_CHCR6, HW_CHCR6, 4), 0u);
    PE_StoreU32(pe_gpu_reg_word(P_MADR6, HW_MADR6, 4), ot + 4u * ((uint32_t)count - 1u));
    PE_StoreU32(pe_gpu_reg_word(P_BCR6, HW_BCR6, 4), (uint32_t)count);
    PE_StoreU32(pe_gpu_reg_word(P_CHCR6, HW_CHCR6, 4), CHCR_START_OTC);
    (void)pe_plat_gpu_seed_deadline_host();
    while (PE_LoadU32(pe_gpu_reg_word(P_CHCR6, HW_CHCR6, 4)) & CHCR_BUSY) {
        if (func_80077404() != 0) return -1;
        if (PE_Port_ShouldStop()) break;
        if (++polls > SPIN_BUDGET) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            break;
        }
    }
    return count;
}
