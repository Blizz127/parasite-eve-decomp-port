/* Synthetic GPU/RAM check against previously recorded original-derived vectors. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_guest_ram.h"
#include "pe_gpu.h"
#include "psx_compat.h"
#include "retail_store_image_cases.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"case %u entry %u failed line %d\n",k,entry,__LINE__); abort(); } } while (0)

static uint32_t gp0[8],gp1[4],dma[3];
static unsigned ngp0,ngp1,ndma,nread,nwait,fail_stage;
static const uint32_t *reads;
static unsigned read_count;
int PE_MMIO_InShadowBlock(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void *PE_MMIO_Translate(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void PE_MMIO_Store(pe_addr_t a,size_t n,uint32_t v) { (void)a;(void)n;(void)v;abort(); }
int pe_plat_gpu_readback_wait_existing(uint32_t ready)
{ if(ready!=(nwait==0?0x04000000u:0x08000000u)) abort(); nwait++;return fail_stage!=nwait; }
uint32_t pe_plat_gpu_seed_deadline_host(void) { return 0; }
int PE_GPU_CanBeginImageLoad(int needs_dma) { (void)needs_dma;return 1; }
int PE_GPU_WriteGP0(uint32_t value) { if(ngp0>=8) abort();gp0[ngp0++]=value;return 1; }
int PE_GPU_WriteGP1(uint32_t value) { if(ngp1>=4) abort();gp1[ngp1++]=value;return 1; }
int PE_GPU_ReadGP0(uint32_t *value)
{ if(!value || nread>=read_count) abort();if(fail_stage==3 || (fail_stage==5 && nread==1)) return 0;*value=reads[nread++];return 1; }
int PE_GPU_DMA2Issue(pe_addr_t address,uint32_t bcr,uint32_t chcr)
{ if(ndma++) abort();dma[0]=address;dma[1]=bcr;dma[2]=chcr;return fail_stage!=4; }
static void reset_events(void) { ngp0=ngp1=ndma=nread=nwait=fail_stage=0; }
int main(void)
{
    const pe_addr_t rect=0x801e0000u,dst=0x800d0000u;
    unsigned k,entry;
    PE_RamInit();
    for (k=0;k<sizeof(NAM10_store_image_cases)/sizeof(NAM10_store_image_cases[0]);k++) {
        uint32_t xy=(uint16_t)NAM10_store_image_cases[k].rect[0]|
                    ((uint32_t)(uint16_t)NAM10_store_image_cases[k].rect[1]<<16);
        uint32_t wh=(uint16_t)NAM10_store_image_cases[k].rect[2]|
                    ((uint32_t)(uint16_t)NAM10_store_image_cases[k].rect[3]<<16);
        uint32_t expected_wh=NAM10_store_image_cases[k].width|
                             ((uint32_t)NAM10_store_image_cases[k].height<<16);
        unsigned prefix=NAM10_store_image_cases[k].count;
        for(entry=0;entry<2;entry++) {
            PE_RamReset();reset_events();
            reads=NAM10_store_image_cases[k].prefix;read_count=prefix;
            PE_StoreU16(0x80095750u,(uint16_t)NAM10_store_image_cases[k].limits[0]);
            PE_StoreU16(0x80095752u,(uint16_t)NAM10_store_image_cases[k].limits[1]);
            PE_StoreU32(rect,xy);PE_StoreU32(rect+4,entry?0xdeadbeefu:wh);
            PE_StoreU32(dst-4,0x89abcdefu);
            for(unsigned i=0;i<16;i++) PE_StoreU32(dst+4*i,0xa5a5a5a5u);
            PE_StoreU32(dst+64,0x76543210u);
            int result=entry?pe_plat_gpu_readback_inline_host(xy,wh,dst):
                             pe_plat_gpu_readback_worker_host(rect,dst);
            CHECK((uint32_t)result==NAM10_store_image_cases[k].result);
            CHECK(PE_LoadU32(rect+4)==(entry?0xdeadbeefu:expected_wh));
            CHECK(PE_LoadU32(dst-4)==0x89abcdefu && PE_LoadU32(dst+64)==0x76543210u);
            for(unsigned i=0;i<16;i++)
                CHECK(PE_LoadU32(dst+4*i)==(i<prefix?reads[i]:0xa5a5a5a5u));
            if(result==-1) {
                CHECK(ngp0==0 && ngp1==0 && ndma==0 && nread==0 && nwait==0);
            } else {
                CHECK(ngp0==4 && gp0[0]==0x01000000u && gp0[1]==0xc0000000u &&
                      gp0[2]==xy && gp0[3]==expected_wh && nread==prefix && nwait==2);
                CHECK(ngp1==(NAM10_store_image_cases[k].dma[2]?2u:1u) && gp1[0]==0x04000000u);
                if(NAM10_store_image_cases[k].dma[2]) {
                    CHECK(gp1[1]==0x04000003u && ndma==1);
                    for(unsigned i=0;i<3;i++) CHECK(dma[i]==NAM10_store_image_cases[k].dma[i]);
                } else CHECK(ndma==0);
            }
        }
    }
    unsigned vector_count=k;
    static const pe_addr_t bad_destinations[]={0u,0x801ffffeu,0x801fffc0u,0xfffffffcu};
    for (unsigned i=0;i<sizeof bad_destinations/sizeof bad_destinations[0];i++) {
        k=vector_count+i;
        for(entry=0;entry<2;entry++) {
            PE_RamReset();reset_events();
            PE_StoreU16(0x80095750u,1024);PE_StoreU16(0x80095752u,512);
            PE_StoreU32(rect,0);PE_StoreU32(rect+4,entry?0xdeadbeefu:(2u<<16)|32u);
            PE_Fill(0x801fffc0u,64u,0xa5u);
            int result=entry?pe_plat_gpu_readback_inline_host(0,(2u<<16)|32u,bad_destinations[i]):
                             pe_plat_gpu_readback_worker_host(rect,bad_destinations[i]);
            CHECK(result==-1 && ngp0==0 && ngp1==0 && ndma==0 && nread==0 && nwait==0);
            CHECK(PE_LoadU32(rect+4)==(entry?0xdeadbeefu:(2u<<16)|32u));
            for(unsigned j=0;j<64;j++) CHECK(PE_LoadU8(0x801fffc0u+j)==0xa5u);
        }
    }
    static const uint32_t read_value=0x12345678u;
    for(unsigned stage=1;stage<=3;stage++) {
        k=vector_count+4+stage;
        for(entry=0;entry<2;entry++) {
            PE_RamReset();reset_events();fail_stage=stage;
            reads=&read_value;read_count=1;
            PE_StoreU16(0x80095750u,1024);PE_StoreU16(0x80095752u,512);
            PE_StoreU32(rect,0);PE_StoreU32(rect+4,entry?0xdeadbeefu:(1u<<16)|1u);
            PE_StoreU32(dst-4,0x89abcdefu);PE_StoreU32(dst,0xa5a5a5a5u);PE_StoreU32(dst+4,0x76543210u);
            int result=entry?pe_plat_gpu_readback_inline_host(0,(1u<<16)|1u,dst):
                             pe_plat_gpu_readback_worker_host(rect,dst);
            CHECK(result==-1 && ndma==0 && nread==0);
            CHECK(PE_LoadU32(dst-4)==0x89abcdefu && PE_LoadU32(dst)==0xa5a5a5a5u && PE_LoadU32(dst+4)==0x76543210u);
            CHECK(nwait==(stage==1?1u:2u) && ngp0==(stage==1?0u:4u) && ngp1==(stage==1?0u:1u));
        }
    }
    static const uint32_t partial_values[]={0x12345678u,0x87654321u};
    for(unsigned stage=4;stage<=5;stage++) {
        k=vector_count+4+stage;
        for(entry=0;entry<2;entry++) {
            PE_RamReset();reset_events();fail_stage=stage;
            reads=partial_values;read_count=2;
            uint32_t wh=(1u<<16)|(stage==4?33u:3u);
            PE_StoreU16(0x80095750u,1024);PE_StoreU16(0x80095752u,512);
            PE_StoreU32(rect,0);PE_StoreU32(rect+4,entry?0xdeadbeefu:wh);
            PE_StoreU32(dst-4,0x89abcdefu);PE_StoreU32(dst,0xa5a5a5a5u);
            PE_StoreU32(dst+4,0xa5a5a5a5u);PE_StoreU32(dst+8,0x76543210u);
            int result=entry?pe_plat_gpu_readback_inline_host(0,wh,dst):
                             pe_plat_gpu_readback_worker_host(rect,dst);
            CHECK(result==-1 && nread==1 && nwait==2 && ngp0==4);
            CHECK(PE_LoadU32(dst-4)==0x89abcdefu && PE_LoadU32(dst)==partial_values[0] &&
                  PE_LoadU32(dst+4)==0xa5a5a5a5u && PE_LoadU32(dst+8)==0x76543210u);
            CHECK(ndma==(stage==4?1u:0u) && ngp1==(stage==4?2u:1u));
        }
    }
    PE_RamDestroy();
    printf("PASS %u original-derived vectors, 4 invalid destinations and 5 backend failures x 2 native entries; scripted GPU\n",vector_count);
    return 0;
}
