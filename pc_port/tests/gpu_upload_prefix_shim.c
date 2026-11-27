/* Test-only services for the independent native GPU upload worker. */
#include "pe_guest_ram.h"
#include "pe_gpu.h"
#include <stdlib.h>

static unsigned gp0_calls, dma_calls;
static uint32_t dma_bcr;
void UploadTest_Reset(void) { gp0_calls=0; dma_calls=0; dma_bcr=0; }
unsigned UploadTest_GP0Calls(void) { return gp0_calls; }
unsigned UploadTest_DMACalls(void) { return dma_calls; }
uint32_t UploadTest_DMABCR(void) { return dma_bcr; }
int pe_plat_gpu_upload_span_existing(pe_addr_t source,uint32_t words)
{ return PE_RangeIsRam(source,(size_t)words*4u); }
int pe_plat_gpu_upload_wait_existing(void) { return 1; }
uint32_t pe_plat_gpuq_seed_deadline(void) { return 0; }
int PE_GPU_CanBeginImageLoad(int needs_dma) { (void)needs_dma; return 1; }
int PE_GPU_WriteGP0(uint32_t value) { (void)value; gp0_calls++; return 1; }
int PE_GPU_WriteGP1(uint32_t value) { (void)value; return 1; }
int PE_GPU_DMA2Issue(pe_addr_t address,uint32_t bcr,uint32_t chcr)
{ (void)address;(void)chcr;dma_calls++;dma_bcr=bcr;return 1; }
int PE_MMIO_InShadowBlock(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void *PE_MMIO_Translate(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void PE_MMIO_Store(pe_addr_t a,size_t n,uint32_t v) { (void)a;(void)n;(void)v;abort(); }
