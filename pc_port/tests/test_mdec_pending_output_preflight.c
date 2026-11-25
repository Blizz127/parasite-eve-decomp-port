/* Synthetic MDEC output preflight with an already-busy DMA; no game data. */
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
#define abort() (fprintf(stderr,"fixture abort at line %d\n",__LINE__),_Exit(134))
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
static void prepare_synthetic_decode(void)
{
    PE_MDEC_Init();PE_MDEC_WriteControl(0x60000000u);
    PE_StoreU32(0x80140000u,0x40000001u);
    for(unsigned i=0;i<128u;i++) PE_StoreU8(0x80140004u+i,16u);
    if(PE_MDEC_SubmitInputTable(0x80140000u,32u)!=0) abort();
    (void)PE_MDEC_Service();
    PE_StoreU32(0x80140000u,0x60000000u);
    for(unsigned i=0;i<64u;i++) PE_StoreU16(0x80140004u+i*2u,(i/8u==i%8u)?16384u:0u);
    if(PE_MDEC_SubmitInputTable(0x80140000u,32u)!=0) abort();
    (void)PE_MDEC_Service();
    PE_StoreU32(0x80144000u,0x28000001u);
    PE_StoreU16(0x80144004u,0x2020u);PE_StoreU16(0x80144006u,0xfe00u);
    if(!PE_MDEC_BeginDecode(0x80144000u) || PE_Port_ShouldStop()) abort();
}
int main(void)
{
    static const struct { uint32_t dst,words;const char *name; } cases[]={
        {0x80160000u,0u,"MDEC_output_block_count"},
        {0x80160000u,31u,"MDEC_output_block_count"},
        {0x80160000u,0x00200000u,"MDEC_output_block_count"},
        {0x801fffc0u,32u,"MDEC_output_table_range"},
        {0x001fffc0u,32u,"MDEC_output_table_range"},
        {0xfffffffcu,32u,"MDEC_output_table_range"},
    };
    PE_RamInit();
    for(unsigned i=0;i<sizeof cases/sizeof cases[0];i++) {
        PE_RamReset();PE_GPU_Init();PE_IRQ_Reset();PE_Port_RunControlReset();Bootstrap_Init();bridge_calls=0;
        PE_GPU_WriteDPCR(0x88u);
        prepare_synthetic_decode();
        bridge_calls=0;
        PE_Fill(0x80150000u,64u,0xa5u);
        PE_MDEC_WriteDMA1(0,0x150000u);
        PE_MDEC_WriteDMA1(4,0x00020008u);
        PE_MDEC_WriteDMA1(8,0x01000200u);
        PeMdecState before,after;PE_MDEC_GetState(&before);
        uint32_t dpcr=PE_GPU_ReadDPCR();uint16_t istat=PE_IRQ_ReadStatus();
        CHECK(!PE_Port_ShouldStop() && (before.dma1_chcr&0x01000000u));
        CHECK(!PE_MDEC_SubmitOutput(cases[i].dst,cases[i].words));
        PE_MDEC_GetState(&after);
        CHECK(PE_Port_ShouldStop() && PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY && PE_Port_StopEpoch()==1 && g_bootstrap_arg_call_count==1 &&
              strcmp(g_bootstrap_arg_calls[0].symbol,cases[i].name)==0 &&
              strcmp(g_bootstrap_arg_calls[0].caller,"MDEC_decode")==0 &&
              g_bootstrap_arg_calls[0].arg0==(i<3?cases[i].words:cases[i].dst));
        CHECK(after.dma1_madr==before.dma1_madr && after.dma1_bcr==before.dma1_bcr &&
              after.dma1_chcr==before.dma1_chcr && after.output_count==before.output_count &&
              after.completed_output_count==before.completed_output_count &&
              PE_GPU_ReadDPCR()==dpcr && PE_IRQ_ReadStatus()==istat && bridge_calls==0);
        for(unsigned j=0;j<64;j++) CHECK(PE_LoadU8(0x80150000u+j)==0xa5u);
        /* After clearing the test stop, one explicit service is productive;
         * the unchanged canary above therefore catches unintended progress. */
        PE_Port_RunControlReset();Bootstrap_Init();
        (void)PE_MDEC_Service();PE_MDEC_GetState(&after);
        CHECK(!PE_Port_ShouldStop() && bridge_calls==1 && after.dma1_bcr==0x00010008u &&
              PE_LoadU8(0x80150000u)==160u);
    }
    {
        unsigned i=6;
        PE_RamReset();PE_GPU_Init();PE_IRQ_Reset();PE_Port_RunControlReset();Bootstrap_Init();bridge_calls=0;
        PE_GPU_WriteDPCR(0x88u);prepare_synthetic_decode();bridge_calls=0;
        PE_Fill(0x80150000u,64u,0xa5u);
        PE_MDEC_WriteDMA1(0,0x150000u);PE_MDEC_WriteDMA1(4,0x00020008u);PE_MDEC_WriteDMA1(8,0x01000200u);
        PeMdecState before,after;PE_MDEC_GetState(&before);
        uint32_t dpcr=PE_GPU_ReadDPCR();uint16_t istat=PE_IRQ_ReadStatus();
        PE_Port_RequestStop(PE_PORT_STOP_EXPLICIT);
        CHECK(!PE_MDEC_SubmitOutput(0x80160000u,32u));
        PE_MDEC_GetState(&after);
        CHECK(PE_Port_ShouldStop() && PE_Port_GetStopReason()==PE_PORT_STOP_EXPLICIT && PE_Port_StopEpoch()==1 && g_bootstrap_arg_call_count==0 && !bridge_calls && (after.dma1_chcr&0x01000000u) &&
              after.dma1_madr==before.dma1_madr && after.dma1_bcr==before.dma1_bcr &&
              after.dma1_chcr==before.dma1_chcr && after.output_count==before.output_count &&
              PE_GPU_ReadDPCR()==dpcr && PE_IRQ_ReadStatus()==istat);
        for(unsigned j=0;j<64;j++) CHECK(PE_LoadU8(0x80150000u+j)==0xa5u);
        PE_Port_RunControlReset();(void)PE_MDEC_Service();PE_MDEC_GetState(&after);
        CHECK(!PE_Port_ShouldStop() && bridge_calls==1 && after.dma1_bcr==0x00010008u && PE_LoadU8(0x80150000u)==160u);
    }
    static const struct { uint32_t src,words;const char *name; } input_cases[]={
        {0x80160000u,0u,"MDEC_input_block_count"},
        {0x80160000u,31u,"MDEC_input_block_count"},
        {0x80160000u,0x00200000u,"MDEC_input_block_count"},
        {0x801fff80u,32u,"MDEC_input_table_range"},
        {0x001fff80u,32u,"MDEC_input_table_range"},
        {0xfffffffcu,32u,"MDEC_input_table_range"},
    };
    for(unsigned i=0;i<14;i++) {
        PE_RamReset();PE_GPU_Init();PE_IRQ_Reset();PE_Port_RunControlReset();Bootstrap_Init();bridge_calls=0;
        PE_GPU_WriteDPCR(0x88u);prepare_synthetic_decode();bridge_calls=0;
        PE_Fill(0x80150000u,64u,0xa5u);PE_Fill(0x80151000u,64u,0xa5u);
        for(unsigned j=0;j<3;j++) PE_StoreU32(0x80151000u+4*j,0);
        PE_MDEC_WriteDMA0(0,0x151000u);PE_MDEC_WriteDMA0(4,0x00030001u);PE_MDEC_WriteDMA0(8,0x01000201u);
        PE_MDEC_WriteDMA1(0,0x150000u);PE_MDEC_WriteDMA1(4,0x00020008u);PE_MDEC_WriteDMA1(8,0x01000200u);
        PeMdecState before,after;PE_MDEC_GetState(&before);
        uint32_t dpcr=PE_GPU_ReadDPCR();uint16_t istat=PE_IRQ_ReadStatus();
        CHECK(!PE_Port_ShouldStop() && before.dma0_active && (before.dma1_chcr&0x01000000u));
        const char *expected_boundary=NULL;
        uint32_t expected_value=0;
        if(i<6) {
            CHECK(!PE_MDEC_SubmitOutput(cases[i].dst,cases[i].words));
            expected_boundary=cases[i].name;
            expected_value=i<3?cases[i].words:cases[i].dst;
        } else if(i<12) {
            CHECK(PE_MDEC_SubmitInputTable(input_cases[i-6].src,input_cases[i-6].words)==-1);
            expected_boundary=input_cases[i-6].name;
            expected_value=i<9?input_cases[i-6].words:input_cases[i-6].src;
        } else {
            PE_Port_RequestStop(PE_PORT_STOP_EXPLICIT);
            if(i==12) CHECK(PE_MDEC_SubmitInputTable(0x80160000u,32u)==-1);
            else CHECK(!PE_MDEC_SubmitOutput(0x80160000u,32u));
        }
        PE_MDEC_GetState(&after);
        CHECK(PE_Port_ShouldStop() && PE_Port_StopEpoch()==1 && bridge_calls==0);
        CHECK(PE_Port_GetStopReason()==(expected_boundary?PE_PORT_STOP_UNRESOLVED_BOUNDARY:PE_PORT_STOP_EXPLICIT));
        if(expected_boundary) CHECK(g_bootstrap_arg_call_count==1 &&
                                    strcmp(g_bootstrap_arg_calls[0].symbol,expected_boundary)==0 &&
                                    strcmp(g_bootstrap_arg_calls[0].caller,"MDEC_decode")==0 &&
                                    g_bootstrap_arg_calls[0].arg0==expected_value);
        else CHECK(g_bootstrap_arg_call_count==0);
        CHECK(after.dma0_madr==before.dma0_madr && after.dma0_bcr==before.dma0_bcr &&
              after.dma0_chcr==before.dma0_chcr && after.dma0_active==before.dma0_active &&
              after.dma1_madr==before.dma1_madr && after.dma1_bcr==before.dma1_bcr &&
              after.dma1_chcr==before.dma1_chcr && after.output_count==before.output_count &&
              after.completed_output_count==before.completed_output_count &&
              after.upload_count==before.upload_count && after.completed_upload_count==before.completed_upload_count &&
              PE_GPU_ReadDPCR()==dpcr && PE_IRQ_ReadStatus()==istat);
        for(unsigned j=0;j<64;j++) CHECK(PE_LoadU8(0x80150000u+j)==0xa5u);
        for(unsigned j=12;j<64;j++) CHECK(PE_LoadU8(0x80151000u+j)==0xa5u);
        PE_Port_RunControlReset();Bootstrap_Init();(void)PE_MDEC_Service();PE_MDEC_GetState(&after);
        CHECK(!PE_Port_ShouldStop() && bridge_calls==1 && after.dma0_bcr==0x00020001u &&
              after.dma1_bcr==0x00010008u && PE_LoadU8(0x80150000u)==160u);
    }
    PE_RamDestroy();
    puts("PASS 6 invalid and 1 pre-stopped output controls plus 14 cross-channel DMA0/DMA1 controls");
    return 0;
}
