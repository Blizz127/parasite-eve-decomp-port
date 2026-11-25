/*
 * Host MDEC I/O wrappers.  Written clean-room from psx-spx and black-box
 * vectors of the original code, with no SDK source.  See pe_plat/mdec_io.h.
 * Self-test mutants for the checker only: -DMDECIO_MUTANT (CHCR value),
 * -DMDECIO_MUTANT_INPUT (command bit), -DMDECIO_MUTANT_DPCR (OR mask 0x80),
 * -DMDECIO_MUTANT_ORDER (swap MADR/BCR write order, row 2),
 * -DMDECIO_MUTANT_WAIT (busy test on bit 28 instead of 29).
 */
#include <stdint.h>
#include "pe_plat/mdec_io.h"

#ifdef MDECIO_MUTANT_DPCR
#define DPCR_MASK 0x80u
#else
#define DPCR_MASK 0x88u /* DMA0 (bit 3) + DMA1 (bit 7) master enables, psx-spx DPCR */
#endif

static void put(pe_mdec_io_result *r, uint32_t reg, uint32_t value)
{
    r->writes[r->count].reg = reg;
    r->writes[r->count].value = value;
    r->count++;
}

static pe_mdec_io_result empty(void)
{
    pe_mdec_io_result r;
    unsigned i;
    for (i = 0; i < PE_MDECIO_MAX_WRITES; i++) { r.writes[i].reg = 0; r.writes[i].value = 0; }
    r.count = 0; r.edited_command = 0; r.upload_source = 0; r.upload_words = 0;
    r.would_wait = 0; r.exit_code = PE_MDECIO_EXIT_OK;
    return r;
}

pe_mdec_io_result pe_mdec_io_input_trace(uint32_t command, uint32_t mode,
                                         uint32_t command_block,
                                         uint32_t dpcr_in, uint32_t mdec_status)
{
    pe_mdec_io_result r = empty();
    uint32_t c1, c2, words;
#ifdef MDECIO_MUTANT_WAIT
    if (mdec_status & 0x10000000u) { r.would_wait = 1; return r; }
#else
    if (mdec_status & PE_MDECIO_STATUS_BUSY) { r.would_wait = 1; return r; }
#endif
    /* step 1: bit 27 (low bit of depth field 28-27) := !(mode & 1) */
    c1 = command & ~0x08000000u;
    if (!(mode & 1u)) c1 |= 0x08000000u;
    /* step 2: bit 25 (bit15 set) := (mode & 2) */
    c2 = c1 & ~0x02000000u;
    if (mode & 2u) c2 |= 0x02000000u;
#ifdef MDECIO_MUTANT_INPUT
    c2 ^= 0x04000000u;
#endif
    words = c2 & 0xFFFFu; /* psx-spx: bits 15-0 parameter words */
    put(&r, command_block, c1);
    put(&r, command_block, c2);
    put(&r, PE_MDECIO_REG_DPCR, dpcr_in | DPCR_MASK);
    put(&r, PE_MDECIO_REG_DMA0_MADR, command_block + 4u);
    put(&r, PE_MDECIO_REG_DMA0_BCR, 0x20u | ((words >> 5) << 16));
    put(&r, PE_MDECIO_REG_MDEC0, c2);
    put(&r, PE_MDECIO_REG_DMA0_CHCR, 0x01000201u);
    r.edited_command = c2;
    r.upload_source = command_block + 4u;
    r.upload_words = words;
    return r;
}

pe_mdec_io_result pe_mdec_io_output_trace(uint32_t destination, uint32_t words,
                                          uint32_t dpcr_in)
{
    pe_mdec_io_result r = empty();
    uint32_t bcr = 0x20u | ((words >> 5) << 16); /* BS 0x20, BA words/32, 32-bit wrap */
#ifdef MDECIO_MUTANT
    uint32_t chcr = 0x01000201u;
#else
    uint32_t chcr = 0x01000200u; /* to RAM, SyncMode 1, start bit 24 */
#endif
    put(&r, PE_MDECIO_REG_DPCR, dpcr_in | DPCR_MASK);
    put(&r, PE_MDECIO_REG_DMA1_CHCR, 0u); /* stop the channel before reprogramming */
#ifdef MDECIO_MUTANT_ORDER
    put(&r, PE_MDECIO_REG_DMA1_BCR, bcr);
    put(&r, PE_MDECIO_REG_DMA1_MADR, destination);
#else
    put(&r, PE_MDECIO_REG_DMA1_MADR, destination);
    put(&r, PE_MDECIO_REG_DMA1_BCR, bcr);
#endif
    put(&r, PE_MDECIO_REG_DMA1_CHCR, chcr);
    return r;
}

pe_mdec_io_result pe_mdec_io_output_guarded(uint32_t destination, uint32_t words,
                                            uint32_t dpcr_in)
{
    uint32_t blocks = words >> 5;
    if (blocks == 0u || blocks > 0xFFFFu) {
        pe_mdec_io_result r = empty();
        r.exit_code = PE_MDECIO_EXIT_BLOCK_COUNT;
        return r;
    }
    return pe_mdec_io_output_trace(destination, words, dpcr_in);
}
