/*
 * pe_plat_mdec_io_host.c - guest adapters for the libpress MDEC I/O wrapper rows 0x8010BFA0 / 0x8010C01C.
 *
 * The register sequences come from platform/pe_plat_mdec_io.c (clean-room, fresh author; verified against the
 * original's write-ordered traces, docs/evidence/native-press-20261125/ and the sw-mdecw report).  This file only
 * applies them through the port's host models: RAM stores (the edited command word) via PE_StoreU32 in list order, and
 * the hardware-register sequence via the host MDEC model's block-level submit calls (PE_MDEC_SubmitInputTable /
 * PE_MDEC_SubmitOutput), after checking that the list's DMA MADR write matches what the submit call will do.
 * The MDEC status register has no host read model ("Status reads remain a boundary", pe_mdec.h), so the busy bit
 * (original waits while status bit 29 is set) is passed as 0; a would_wait result therefore cannot occur and, should
 * it ever be returned, ends in a named boundary stop instead of a spin.
 */
#include "pe_plat/mdec_io_host.h"
#include "pe_plat/mdec_io.h"
#include "pe_guest_ram.h"
#include "pe_gpu.h"
#include "pe_mdec.h"
#include "game_port.h"
#include "pe_bootstrap.h"

/* RAM stores of a write list are applied in order; the hardware-register sequences (DPCR, DMA0 MADR/BCR/MDEC0/CHCR for
 * the input row, DPCR + DMA1 CHCR/MADR/BCR/CHCR for the output row) are performed by the host model's block-level
 * submit calls, which implement exactly those sequences (DPCR |= 0x88, MADR, BCR, command, start) including servicing of
 * an older pending DMA; writing the registers one by one through PE_MDEC_WriteDMA0/1 would hit the model's
 * "MDEC_DMA0_busy_register" boundary on back-to-back submissions (found by the suite). */
static int CheckShape(const pe_mdec_io_result *r, uint32_t first_hw, uint32_t madr_reg, uint32_t madr_value)
{
    /* sanity: the list's MADR write must be the one the submit call will make */
    for (uint32_t i = first_hw; i < r->count; i++)
        if (r->writes[i].reg == madr_reg) return r->writes[i].value == madr_value;
    return 0;
}

void pe_plat_mdec_input_host(uint32_t command_block, uint32_t mode)
{
    pe_mdec_io_result r = pe_mdec_io_input_trace(PE_LoadU32(command_block), mode, command_block, PE_GPU_ReadDPCR(), 0u);
    uint32_t i;
    if (r.would_wait) {
        Bootstrap_ReturnVoid1("MDEC_in_sync_wait", "MDEC_decode", command_block);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }
    for (i = 0; i < r.count && r.writes[i].reg >= 0x80000000u; i++) PE_StoreU32(r.writes[i].reg, r.writes[i].value);
    if (!CheckShape(&r, i, PE_MDECIO_REG_DMA0_MADR, r.upload_source)) {
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }
    (void)PE_MDEC_SubmitInputTable(command_block, r.upload_words);
}

void pe_plat_mdec_output_host(uint32_t destination, uint32_t words)
{
    pe_mdec_io_result r = pe_mdec_io_output_guarded(destination, words, PE_GPU_ReadDPCR());
    if (r.exit_code == PE_MDECIO_EXIT_BLOCK_COUNT) {
        Bootstrap_ReturnVoid1("MDEC_output_block_count", "MDEC_decode", words);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }
    if (!CheckShape(&r, 0, PE_MDECIO_REG_DMA1_MADR, destination)) {
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }
    (void)PE_MDEC_SubmitOutput(destination, words);
}
