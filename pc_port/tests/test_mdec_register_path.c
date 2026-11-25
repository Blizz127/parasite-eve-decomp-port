/* MDEC register path == block-level API: a reset command and a table-upload sequence arriving through
 * PE_MDEC_WriteControl / WriteCommandData / WriteDMA0 leave the model with the same state and bookkeeping
 * (reset_count, upload history, upload_count, input_wait_count, DMA registers, completed uploads) as
 * PE_MDEC_BeginReset + PE_MDEC_SubmitInputTable for the same blocks.  Standalone; build and run from the repo root:
 *   cc -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tests/test_mdec_register_path.c pc_port/platform/pe_mdec.c -o t && ./t
 * pe_mdec.c neighbours are stubbed (same stub set as test_mdec_status_word.c). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pe_guest_ram.h"
#include "pe_gpu.h"
#include "pe_irq.h"
#include "pe_irq_delivery.h"
#include "pe_mdec.h"
#include "pe_bootstrap.h"
#include "game_port.h"
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d failed\n",__LINE__); abort(); } } while(0)
static uint32_t dpcr;
static uint8_t ram[0x200000];
void Bootstrap_ReturnVoid1(const char *s, const char *c, uint32_t a) { (void)s; (void)c; (void)a; }
uint32_t PE_GPU_ReadDPCR(void) { return dpcr; }
void PE_GPU_WriteDPCR(uint32_t v) { dpcr = v; }
uint32_t PE_GPU_ReadDICR(void) { return 0; }
int PE_GPU_LatchDMACompletionFlag(uint32_t c) { (void)c; return 0; }
PeIrqEdgeResult PE_IRQ_BridgeDICRRisingEdge(PeIrqGeneration g) { (void)g; return PE_IRQ_EDGE_NONE; }
PeIrqGeneration PE_IRQ_Generation(void) { return 0; }
uint32_t PE_LoadU32(pe_addr_t a) { uint32_t v; memcpy(&v, ram + (a & 0x1FFFFF), 4); return v; }
uint16_t PE_LoadU16(pe_addr_t a) { uint16_t v; memcpy(&v, ram + (a & 0x1FFFFF), 2); return v; }
void PE_StoreU32(pe_addr_t a, uint32_t v) { memcpy(ram + (a & 0x1FFFFF), &v, 4); }
void PE_StoreU8(pe_addr_t a, uint8_t v) { ram[a & 0x1FFFFF] = v; }
bool PE_RangeIsRam(pe_addr_t a, size_t n) { return (a & 0x1FFFFF) + n <= sizeof ram; }
pe_addr_t PE_RamCanonical(pe_addr_t a, size_t n) { (void)n; return a & 0x1FFFFF; }
const void *PE_TranslateConst(pe_addr_t a, size_t n) { (void)n; return ram + (a & 0x1FFFFF); }
void PE_Port_RequestStop(PEPortStopReason r) { (void)r; abort(); }
int PE_Port_ShouldStop(void) { return 0; }
static void seed(unsigned with_overlay)
{
    memset(ram, 0, sizeof ram);
    if (!with_overlay) return;
    PE_StoreU32(0x8010DA0Cu, 0x40000001u);
    for (unsigned i = 1; i < 17; i++) PE_StoreU32(0x8010DA0Cu + 4 * i, 0x01010101u * i);
    PE_StoreU32(0x8010DA90u, 0x60000000u);
    for (unsigned i = 1; i < 33; i++) PE_StoreU32(0x8010DA90u + 4 * i, 0x00020001u * i);
}
static void upload_regs(uint32_t block)
{
    while (PE_MDEC_StatusWord()) (void)PE_MDEC_Service();
    dpcr |= 0x88u;
    PE_MDEC_WriteDMA0(0, block + 4u);
    PE_MDEC_WriteDMA0(4, 0x20u | (1u << 16));
    (void)PE_MDEC_WriteCommandData(PE_LoadU32(block));
    PE_MDEC_WriteDMA0(8, 0x01000201u);
}
static void same(const PeMdecState *a, const PeMdecState *b)
{
    CHECK(a->reset_count == b->reset_count && a->upload_count == b->upload_count &&
          a->input_wait_count == b->input_wait_count && a->completed_upload_count == b->completed_upload_count &&
          a->dma0_active == b->dma0_active && a->dma0_madr == b->dma0_madr && a->dma0_bcr == b->dma0_bcr &&
          a->dma0_chcr == b->dma0_chcr && a->dma1_chcr == b->dma1_chcr && a->control_last_write == b->control_last_write &&
          a->command_last_write == b->command_last_write && a->control_write_count == b->control_write_count);
    for (unsigned i = 0; i < PE_MDEC_UPLOAD_HISTORY; i++)
        CHECK(a->uploads[i].command == b->uploads[i].command && a->uploads[i].source == b->uploads[i].source &&
              a->uploads[i].word_count == b->uploads[i].word_count && a->uploads[i].bcr == b->uploads[i].bcr &&
              a->uploads[i].payload_fnv1a64 == b->uploads[i].payload_fnv1a64);
}
int main(void)
{
    for (unsigned overlay = 0; overlay < 2; overlay++) {
        PeMdecState reg, blk;
        for (unsigned round = 0; round < 3; round++) {          /* repeated resets keep restarting the telemetry */
            seed(overlay); dpcr = 0; PE_MDEC_Init();
            for (unsigned r = 0; r <= round; r++) {
                PE_MDEC_WriteControl(0x80000000u);
                PE_MDEC_WriteDMA0(8, 0u); PE_MDEC_WriteDMA1(8, 0u);
                PE_MDEC_WriteControl(0x60000000u);
                upload_regs(0x8010DA0Cu); upload_regs(0x8010DA90u);
            }
            PE_MDEC_GetState(&reg);
            seed(overlay); dpcr = 0; PE_MDEC_Init();
            for (unsigned r = 0; r <= round; r++) {
                PE_MDEC_BeginReset(); PE_MDEC_ClearDmaChannels(); PE_MDEC_WriteControl(0x60000000u);
                CHECK(PE_MDEC_SubmitInputTable(0x8010DA0Cu, 0x20u) == 0);
                CHECK(PE_MDEC_SubmitInputTable(0x8010DA90u, 0x20u) == 0);
            }
            PE_MDEC_GetState(&blk);
            same(&reg, &blk);
            CHECK(reg.reset_count == round + 1u && reg.upload_count == 2u);
        }
    }
    printf("PASS\n");
    return 0;
}
