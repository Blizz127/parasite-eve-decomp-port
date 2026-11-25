/* Scripted GPU/readiness services for bounded native readback comparisons. */
#include "pe_guest_ram.h"
#include "pe_gpu.h"
#include <stdlib.h>
static struct { uint32_t kind,address,value; } events[40];
static uint32_t read_values[16];
static unsigned event_count,read_count,read_at;
static void event(uint32_t k,uint32_t a,uint32_t v)
{ if(event_count>=40) abort();events[event_count].kind=k;events[event_count].address=a;events[event_count++].value=v; }
void ReadEvents_Reset(void) { event_count=read_count=read_at=0; }
void ReadEvents_AddValue(uint32_t v) { if(read_count>=16) abort();read_values[read_count++]=v; }
unsigned ReadEvents_Count(void) { return event_count; }
uint32_t ReadEvents_Kind(unsigned i) { if(i>=event_count) abort();return events[i].kind; }
uint32_t ReadEvents_Address(unsigned i) { if(i>=event_count) abort();return events[i].address; }
uint32_t ReadEvents_Value(unsigned i) { if(i>=event_count) abort();return events[i].value; }
int pe_plat_gpu_readback_wait_existing(uint32_t ready)
{ if(ready!=0x04000000u && ready!=0x08000000u) abort();event(0,0x1f801814u,ready==0x04000000u?0x04000000u:0x0c000000u);return 1; }
uint32_t pe_plat_gpu_seed_deadline_host(void) { return 0; }
int PE_GPU_CanBeginImageLoad(int needs_dma) { (void)needs_dma;return 1; }
int PE_GPU_WriteGP0(uint32_t value) { event(1,0x1f801810u,value);return 1; }
int PE_GPU_WriteGP1(uint32_t value) { event(1,0x1f801814u,value);return 1; }
int PE_GPU_ReadGP0(uint32_t *value)
{ if(!value || read_at>=read_count) abort();*value=read_values[read_at++];event(0,0x1f801810u,*value);return 1; }
int PE_GPU_DMA2Issue(pe_addr_t address,uint32_t bcr,uint32_t chcr)
{ event(1,0x1f8010a0u,address);event(1,0x1f8010a4u,bcr);event(1,0x1f8010a8u,chcr);return 1; }
int PE_MMIO_InShadowBlock(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void *PE_MMIO_Translate(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void PE_MMIO_Store(pe_addr_t a,size_t n,uint32_t v) { (void)a;(void)n;(void)v;abort(); }
