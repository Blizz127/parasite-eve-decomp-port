/* Synthetic actor-shadow integration; no disc, graphics device or game. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "game_port.h"
#include "pe_guest_ram.h"
#include "pe_mmio.h"
#include "pe_sdk.h"

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
extern int32_t func_80068014(pe_addr_t);
static void run(unsigned case_id, int mx, int mz)
{
    PE_RamReset(); memset(&g_pe_gte,0,sizeof g_pe_gte);
    normal_calls = projections = geometry = 0;
    PE_StoreU32(0x801401b4u,0x80141000u); PE_StoreU16(0x80141014u,16);
    PE_StoreU8(0x80140252u,1); PE_StoreU32(0x80140238u,0x80142000u);
    PE_StoreU16(0x8014002eu,9); PE_StoreU32(0x80140278u,0x80145000u);
    PE_StoreU8(0x8014027du,128);
    for (unsigned i=0;i<9;i++) {
        PE_StoreU16(0x80142000u+2*i,i==0||i==4?4096:i==2?(uint16_t)mx:i==8?(uint16_t)mz:0);
        PE_StoreU16(0x800b89f8u+2*i,(i==0||i==4||i==8)?4096:0);
    }
    /* Invented constant values, not the game's table. */
    for (unsigned i=0;i<256;i++) PE_StoreU16(0x800961d0u+2*i,4096);
    PE_StoreU32(0x80142014u,11); PE_StoreU32(0x8014201cu,13);
    PE_StoreU32(0x800b8a14u,512); PE_StoreU32(0x800b8a18u,256);
    PE_StoreU32(0x800b0e38u,0x80146000u);
    for(unsigned i=0;i<4096;i++) PE_StoreU32(0x80146000u+4*i,0xaabbccddu);
    for (unsigned i=0;i<40;i++) PE_StoreU8(0x80145000u+i,0x5a);
    for (unsigned i=0;i<64;i++) PE_StoreU8(0x1f800340u+i,(uint8_t)i);
    if (func_80068014(0x80140000u) || geometry!=1 || projections!=4 ||
        normal_calls!=1) abort();
    char path[160];
    if (snprintf(path,sizeof path,"build/codex-shadow-orientations-20261107/ot-%u.bin",case_id) >= (int)sizeof path) abort();
    FILE *out=fopen(path,"wb"); if (!out) abort();
    for(unsigned i=0;i<16384;i++) if(fputc(PE_LoadU8(0x80146000u+i),out)==EOF) abort();
    if(fclose(out)) abort();
    uint64_t h=UINT64_C(14695981039346656037);
    printf("case=%u packet=",case_id);
    for(unsigned i=0;i<40;i++) printf("%02x",PE_LoadU8(0x80145000u+i));
    for(unsigned i=0;i<16384;i++) { h ^= PE_LoadU8(0x80146000u+i); h *= UINT64_C(1099511628211); }
    printf(" ot=%016llx\n",(unsigned long long)h);
    for (unsigned i=0;i<64;i++) if (PE_LoadU8(0x1f800340u+i)!=(uint8_t)i) abort();
}
int main(void)
{
    static const int v[][2]={{4096,0},{-4096,0},{4096,4096},
                            {-2048,4096},{1024,-4096},{-4096,-4096}};
    for(unsigned i=0;i<sizeof(v)/sizeof(v[0]);i++) run(i,v[i][0],v[i][1]);
    PE_RamDestroy();
    return 0;
}
