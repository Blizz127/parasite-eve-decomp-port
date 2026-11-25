/* Synthetic actor-shadow integration; no disc, graphics device or game. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game_port.h"
#include "pe_guest_ram.h"
#include "pe_mmio.h"
#include "pe_sdk.h"
#include "pe_gpu.h"

int PE_MMIO_InShadowBlock(pe_addr_t a, size_t n) { (void)a; (void)n; return 0; }
void *PE_MMIO_Translate(pe_addr_t a, size_t n) { (void)a; (void)n; abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a, size_t n) { (void)a; (void)n; abort(); }
void PE_MMIO_Store(pe_addr_t a, size_t n, uint32_t v) { (void)a; (void)n; (void)v; abort(); }
void PE_Port_RequestStop(PEPortStopReason r) { (void)r; abort(); }
void Trace_Direct(const char *s) { (void)s; }
static unsigned normal_calls, projections, geometry;
uint32_t __real_pe_plat_gte_vector_normal(uint32_t, uint32_t);
uint32_t __wrap_pe_plat_gte_vector_normal(uint32_t in, uint32_t out)
{
    normal_calls++;
    uint32_t sum = __real_pe_plat_gte_vector_normal(in, out);
    if (sum != 16777216u || PE_LoadU32(out) != 0 ||
        PE_LoadU32(out+4) != 0 || PE_LoadU32(out+8) != 4096) abort();
    return sum;
}
void __real_pe_plat_gte_set_geom_screen(int);
void __wrap_pe_plat_gte_set_geom_screen(int h) { if (h != 256) abort(); geometry++; __real_pe_plat_gte_set_geom_screen(h); }
int32_t __real_pe_plat_gte_rot_trans_pers(uint32_t,uint32_t,uint32_t,uint32_t);
int32_t __wrap_pe_plat_gte_rot_trans_pers(uint32_t v,uint32_t xy,uint32_t p,uint32_t flag)
{
    projections++;
    int32_t depth=__real_pe_plat_gte_rot_trans_pers(v,xy,p,flag);
    return depth;
}
static void upload(unsigned x,unsigned y,unsigned w,unsigned h,uint16_t value)
{
    if(!PE_GPU_WriteGP0(0xa0000000u) || !PE_GPU_WriteGP0((y<<16)|x) ||
       !PE_GPU_WriteGP0((h<<16)|w)) abort();
    uint32_t pair=(uint32_t)value|((uint32_t)value<<16);
    for(unsigned i=0;i<(w*h+1u)/2u;i++) if(!PE_GPU_WriteGP0(pair)) abort();
}
static uint16_t pixel(unsigned x,unsigned y)
{
    uint16_t v; if(!PE_GPU_ReadVRAM(x,y,&v)) abort(); return v;
}
extern int32_t func_80068014(pe_addr_t);
static void run(int nonzero,int transparent)
{
    PE_RamReset(); memset(&g_pe_gte,0,sizeof g_pe_gte);
    normal_calls = projections = geometry = 0;
    PE_StoreU32(0x801401b4u,0x80141000u); PE_StoreU16(0x80141014u,16);
    PE_StoreU8(0x80140252u,1); PE_StoreU32(0x80140238u,0x80142000u);
    PE_StoreU16(0x8014002eu,64); PE_StoreU32(0x80140278u,0x80145000u);
    PE_StoreU8(0x8014027du,128);
    for (unsigned i=0;i<9;i++) {
        PE_StoreU16(0x80142000u+2*i,(i==0||i==4||(i==8&&nonzero))?4096:0);
        PE_StoreU16(0x800b89f8u+2*i,(i==0||i==4||i==8)?4096:0);
    }
    /* Invented constant values, not the game's table. */
    for (unsigned i=0;i<256;i++) PE_StoreU16(0x800961d0u+2*i,4096);
    PE_StoreU32(0x80142014u,80); PE_StoreU32(0x8014201cu,13);
    PE_StoreU32(0x800b8a14u,512); PE_StoreU32(0x800b8a18u,256);
    PE_StoreU32(0x800b0e38u,0x80146000u);
    for(unsigned i=0;i<4096;i++) PE_StoreU32(0x80146000u+4*i,0xaabbccddu);
    for (unsigned i=0;i<40;i++) PE_StoreU8(0x80145000u+i,0x5a);
    for (unsigned i=0;i<64;i++) PE_StoreU8(0x1f800340u+i,(uint8_t)i);
    if (func_80068014(0x80140000u) || geometry!=1 || projections!=4 ||
        normal_calls!=(unsigned)nonzero) abort();
    PE_GPU_Init();
    if(!PE_GPU_WriteGP0(0xe3000000u) || !PE_GPU_WriteGP0(0xe407ffffu)) abort();
    upload(0,0,100,100,0x7fff);
    upload(704,64,32,64,0x0101);
    upload(257,456,1,1,transparent?0u:0xffffu);
    if(pixel(5,5)!=0x7fff || pixel(704,64)!=0x0101 ||
       pixel(257,456)!=(transparent?0u:0xffffu)) abort();
    for(unsigned i=0;i<9;i++) if(!PE_GPU_WriteGP0(PE_LoadU32(0x80145004u+4*i))) abort();
    unsigned changed=0;
    for(unsigned y=0;y<100;y++) for(unsigned x=0;x<100;x++) if(pixel(x,y)!=0x7fff) changed++;
    if(changed!=(transparent?0u:31u)) abort();
    if(pixel(30,30)!=(transparent?0x7fffu:0xc210u) || pixel(20,20)!=0x7fffu) abort();
    PeGpuState state; PE_GPU_GetState(&state);
    if(state.polygon_count!=1u || state.gp0_state!=PE_GPU_GP0_IDLE) abort();
    printf("facing=%d transparent=%d changed=%u\n",nonzero,transparent,changed);
    for (unsigned i=0;i<64;i++) if (PE_LoadU8(0x1f800340u+i)!=(uint8_t)i) abort();
}
int main(void)
{
    run(0,0); run(1,0); run(1,1); PE_RamDestroy();

    return 0;
}
