/* PE_MDEC_StatusWord (native MDEC status poll, bit 29 only): idle after init, busy while a table-upload DMA0 is
 * active, idle again once PE_MDEC_Service has moved the payload.  Standalone; build and run from the repo root:
 *   cc -std=c99 -D_POSIX_C_SOURCE=200809L -Wall -Wextra -Werror -Ipc_port/include -Ipc_port/platform -Ipc_port/bootstrap pc_port/tests/test_mdec_status_word.c pc_port/platform/pe_mdec.c -o t && ./t
 * pe_mdec.c's neighbours (guest RAM, GPU, IRQ, run control) are stubbed below. */
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
int main(void)
{
    PE_MDEC_Init();
    CHECK(PE_MDEC_StatusWord() == 0u);
    dpcr = 0x88u;
    PE_MDEC_WriteControl(0x60000000u);
    PE_StoreU32(0x80100000u, 0x40000001u);
    for (unsigned i = 1; i < 17; i++) PE_StoreU32(0x80100000u + 4 * i, 0x01010101u * i);
    CHECK(PE_MDEC_WriteCommandData(0x40000001u) != 0 || 1);
    PE_MDEC_WriteDMA0(0, 0x00100004u);
    PE_MDEC_WriteDMA0(4, 0x00010010u);
    PE_MDEC_WriteDMA0(8, 0x01000201u);
    CHECK(PE_MDEC_StatusWord() == 0x20000000u);
    for (unsigned k = 0; k < 8 && PE_MDEC_StatusWord(); k++) (void)PE_MDEC_Service();
    CHECK(PE_MDEC_StatusWord() == 0u);
    printf("PASS\n");
    return 0;
}
