/* Scripted GPU and guest RAM services for isolated native upload tests. */
#include "pe_guest_ram.h"
#include "pe_gpu.h"
#include <stdlib.h>
static struct { uint32_t address,value; } events[32];
static unsigned event_count;
static void event(uint32_t a,uint32_t v)
{ if(event_count>=32) abort(); events[event_count].address=a;events[event_count++].value=v; }
void UploadEvents_Reset(void) { event_count=0; }
unsigned UploadEvents_Count(void) { return event_count; }
uint32_t UploadEvents_Address(unsigned i) { if(i>=event_count) abort();return events[i].address; }
uint32_t UploadEvents_Value(unsigned i) { if(i>=event_count) abort();return events[i].value; }
int pe_plat_gpu_upload_span_existing(pe_addr_t source,uint32_t words)
{ return PE_RangeIsRam(source,(size_t)words*4u); }
int pe_plat_gpu_upload_wait_existing(void) { return 1; }
uint32_t pe_plat_gpuq_seed_deadline(void) { return 0; }
int PE_GPU_CanBeginImageLoad(int needs_dma) { (void)needs_dma;return 1; }
int PE_GPU_WriteGP0(uint32_t value) { event(0x1f801810u,value);return 1; }
int PE_GPU_WriteGP1(uint32_t value) { event(0x1f801814u,value);return 1; }
int PE_GPU_DMA2Issue(pe_addr_t address,uint32_t bcr,uint32_t chcr)
{ event(0x1f8010a0u,address);event(0x1f8010a4u,bcr);event(0x1f8010a8u,chcr);return 1; }
int PE_MMIO_InShadowBlock(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void *PE_MMIO_Translate(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void PE_MMIO_Store(pe_addr_t a,size_t n,uint32_t v) { (void)a;(void)n;(void)v;abort(); }
