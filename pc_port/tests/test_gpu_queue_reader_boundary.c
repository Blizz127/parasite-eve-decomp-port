/* Synthetic pump boundary around the standalone, unbound queue reader. */
#include "pe_gpu_queue_ring.h"
#include "pe_guest_ram.h"
#include "pe_gpu.h"
#include "psx_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x) do { if (!(x)) { fprintf(stderr,"failed line %d\n",__LINE__); abort(); } } while (0)
int PE_MMIO_InShadowBlock(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void *PE_MMIO_Translate(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void PE_MMIO_Store(pe_addr_t a,size_t n,uint32_t v) { (void)a;(void)n;(void)v;abort(); }
static uint16_t mask;
static unsigned stop_count, worker_count;
static uint64_t order;
static PeGpuPumpTrace trace;
uint32_t PE_GPU_ReadDMA2CHCR(void) { return 0; }
uint32_t PE_GPU_ReadStatus(void) { return 0x04000000u; }
uint16_t pe_plat_irq_set_mask(uint16_t next) { uint16_t old=mask; mask=next; return old; }
uint32_t pe_plat_dma_callback(uint32_t channel,uint32_t callback)
{ (void)channel;(void)callback;abort(); }
PeGpuPumpTrace *pe_plat_pump_trace_existing(void) { return &trace; }
void pe_plat_pump_enter_existing(void) { }
uint64_t pe_plat_pump_next_existing(void) { return ++order; }
int pe_plat_pump_load_existing(uint32_t index,uint32_t field,uint32_t *out)
{
    if (PE_GpuQueue_ReadWord(index,field,out)) return 1;
    stop_count++;
    return 0;
}
int pe_plat_pump_worker_existing(pe_addr_t worker,pe_addr_t arg,uint32_t aux,uint32_t consumer)
{
    CHECK(worker==0x8017f000u && arg==0x8017f100u && aux==0x11223344u);
    CHECK(consumer==0 && mask==0);
    worker_count++;
    return 1;
}
int pe_plat_pump_drawsync_existing(pe_addr_t callback) { (void)callback; abort(); }
static void run(int invalid)
{
    PE_RamReset();
    memset(&trace,0,sizeof trace);
    order=stop_count=worker_count=0;
    mask=0x1234;
    PE_StoreU32(0x80095874u,invalid?2u:1u);
    PE_StoreU32(0x80095878u,invalid?0x10000u:0u);
    PE_StoreU32(0x80095758u,0x8017f200u);
    PE_StoreU32(0x800bd030u,0x8017f000u);
    PE_StoreU32(0x800bd034u,0x8017f100u);
    PE_StoreU32(0x800bd038u,0x11223344u);
    int returned=99;
    int result=PE_func_80076EE4_Pump(&returned);
    CHECK(result==0);
    if (invalid) {
        CHECK(returned==0 && stop_count==1 && worker_count==0);
        CHECK(mask==0 && PE_LoadU32(0x80095878u)==0x10000u);
        CHECK(PE_LoadU32(0x80095880u)==0x1234u && trace.mask_restore_order==0);
        puts("invalid raw index: fixture stop, consumer unchanged, mask disabled");
    } else {
        CHECK(returned==1 && stop_count==0 && worker_count==1);
        CHECK(mask==0x1234 && PE_LoadU32(0x80095878u)==1);
        puts("valid entry: one worker, consumer advanced, mask restored");
    }
}
int main(void) { run(0); run(1); PE_RamDestroy(); return 0; }
