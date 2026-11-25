/* Synthetic MDEC input preflight with a productive pending DMA0. */
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
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"case %u failed line %d\n",i,__LINE__);abort(); } } while(0)
static unsigned bridge_calls;
int PE_MMIO_InShadowBlock(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void *PE_MMIO_Translate(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void PE_MMIO_Store(pe_addr_t a,size_t n,uint32_t v) { (void)a;(void)n;(void)v;abort(); }
/* Run-control reset dependency is isolated; no stream/device is started. */
void PE_StrFeed_Reset(void) { }
PeIrqEdgeResult PE_IRQ_BridgeDICRRisingEdge(PeIrqGeneration generation)
{ (void)generation;bridge_calls++;return PE_IRQ_EDGE_NONE; }
int main(void)
{
    static const struct { uint32_t src,words;const char *name; } cases[]={
        {0x80160000u,0u,"MDEC_input_block_count"},
        {0x80160000u,31u,"MDEC_input_block_count"},
        {0x80160000u,0x00200000u,"MDEC_input_block_count"},
        {0x801fff80u,32u,"MDEC_input_table_range"},
        {0x001fff80u,32u,"MDEC_input_table_range"},
        {0xfffffffcu,32u,"MDEC_input_table_range"},
    };
    PE_RamInit();
    for(unsigned i=0;i<sizeof cases/sizeof cases[0];i++) {
        PE_RamReset();PE_GPU_Init();PE_IRQ_Reset();PE_MDEC_Init();PE_Port_RunControlReset();Bootstrap_Init();bridge_calls=0;
        PE_GPU_WriteDPCR(8u);PE_MDEC_WriteControl(0x40000000u);
        PE_Fill(0x80150000u,64u,0xa5u);
        for(unsigned j=0;j<3;j++) PE_StoreU32(0x80150000u+4*j,0);
        PE_MDEC_WriteDMA0(0,0x150000u);PE_MDEC_WriteDMA0(4,0x00030001u);PE_MDEC_WriteDMA0(8,0x01000201u);
        PeMdecState before,after;PE_MDEC_GetState(&before);
        uint32_t dpcr=PE_GPU_ReadDPCR();uint16_t istat=PE_IRQ_ReadStatus();
        CHECK(!PE_Port_ShouldStop() && before.dma0_active && before.dma0_bcr==0x00030001u);
        CHECK(PE_MDEC_SubmitInputTable(cases[i].src,cases[i].words)==-1);
        PE_MDEC_GetState(&after);
        CHECK(PE_Port_ShouldStop() && PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY && PE_Port_StopEpoch()==1 && g_bootstrap_arg_call_count==1 &&
              strcmp(g_bootstrap_arg_calls[0].symbol,cases[i].name)==0 &&
              strcmp(g_bootstrap_arg_calls[0].caller,"MDEC_decode")==0 &&
              g_bootstrap_arg_calls[0].arg0==(i<3?cases[i].words:cases[i].src));
        CHECK(after.dma0_madr==before.dma0_madr && after.dma0_bcr==before.dma0_bcr &&
              after.dma0_chcr==before.dma0_chcr && after.dma0_active==before.dma0_active &&
              after.upload_count==before.upload_count && after.completed_upload_count==before.completed_upload_count &&
              PE_GPU_ReadDPCR()==dpcr && PE_IRQ_ReadStatus()==istat && bridge_calls==0);
        for(unsigned j=12;j<64;j++) CHECK(PE_LoadU8(0x80150000u+j)==0xa5u);
        /* Clearing the test stop and explicitly servicing proves this pending
         * request can make progress from the state preserved above. */
        PE_Port_RunControlReset();Bootstrap_Init();(void)PE_MDEC_Service();PE_MDEC_GetState(&after);
        CHECK(!PE_Port_ShouldStop() && bridge_calls==1 && after.dma0_active &&
              after.dma0_bcr==0x00020001u && after.dma0_madr==0x150004u);
    }
    {
        unsigned i=6;
        PE_RamReset();PE_GPU_Init();PE_IRQ_Reset();PE_MDEC_Init();PE_Port_RunControlReset();Bootstrap_Init();bridge_calls=0;
        PE_GPU_WriteDPCR(8u);PE_MDEC_WriteControl(0x40000000u);
        PE_Fill(0x80150000u,64u,0xa5u);
        for(unsigned j=0;j<3;j++) PE_StoreU32(0x80150000u+4*j,0);
        PE_MDEC_WriteDMA0(0,0x150000u);PE_MDEC_WriteDMA0(4,0x00030001u);PE_MDEC_WriteDMA0(8,0x01000201u);
        PeMdecState before,after;PE_MDEC_GetState(&before);
        uint32_t dpcr=PE_GPU_ReadDPCR();uint16_t istat=PE_IRQ_ReadStatus();
        PE_Port_RequestStop(PE_PORT_STOP_EXPLICIT);
        CHECK(PE_MDEC_SubmitInputTable(0x80160000u,32u)==-1);
        PE_MDEC_GetState(&after);
        CHECK(PE_Port_ShouldStop() && PE_Port_GetStopReason()==PE_PORT_STOP_EXPLICIT && PE_Port_StopEpoch()==1 && g_bootstrap_arg_call_count==0 && !bridge_calls && after.dma0_active &&
              after.dma0_madr==before.dma0_madr && after.dma0_bcr==before.dma0_bcr &&
              after.dma0_chcr==before.dma0_chcr && after.upload_count==before.upload_count &&
              PE_GPU_ReadDPCR()==dpcr && PE_IRQ_ReadStatus()==istat);
        for(unsigned j=12;j<64;j++) CHECK(PE_LoadU8(0x80150000u+j)==0xa5u);
        PE_Port_RunControlReset();(void)PE_MDEC_Service();PE_MDEC_GetState(&after);
        CHECK(!PE_Port_ShouldStop() && bridge_calls==1 && after.dma0_bcr==0x00020001u);
    }
    PE_RamDestroy();
    puts("PASS 6 invalid and 1 pre-stopped MDEC input requests preserve productive pending DMA0");
    return 0;
}
