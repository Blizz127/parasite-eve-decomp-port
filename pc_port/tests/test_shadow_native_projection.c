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
    static const uint32_t expected_xy[4] = {0x0004000cu,0x0004fffdu,0x0004000du,0x0004fffdu};
    static const int expected_depth[4] = {135,135,127,127};
    if (projections > 4 || PE_LoadU32(xy) != expected_xy[projections-1] ||
        depth != expected_depth[projections-1] || PE_LoadU32(flag) != 0) abort();
    return depth;
}
extern int32_t func_80068014(pe_addr_t);
static void run(int nonzero)
{
    PE_RamReset(); memset(&g_pe_gte,0,sizeof g_pe_gte);
    normal_calls = projections = geometry = 0;
    PE_StoreU32(0x801401b4u,0x80141000u); PE_StoreU16(0x80141014u,16);
    PE_StoreU8(0x80140252u,1); PE_StoreU32(0x80140238u,0x80142000u);
    PE_StoreU16(0x8014002eu,9); PE_StoreU32(0x80140278u,0x80145000u);
    PE_StoreU8(0x8014027du,128);
    for (unsigned i=0;i<9;i++) {
        PE_StoreU16(0x80142000u+2*i,(i==0||i==4||(i==8&&nonzero))?4096:0);
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
        normal_calls!=(unsigned)nonzero) abort();
    static const uint8_t expected[40] = {
        0xdd,0xcc,0xbb,0x09,0x40,0x40,0x40,0x2e,0x0c,0x00,0x04,0x00,
        0x00,0x40,0x10,0x72,0xfd,0xff,0x04,0x00,0x3f,0x40,0xcb,0x00,
        0x0d,0x00,0x04,0x00,0x00,0x7f,0x5a,0x5a,0xfd,0xff,0x04,0x00,
        0x3f,0x7f,0x5a,0x5a};
    for (unsigned i=0;i<40;i++) if (PE_LoadU8(0x80145000u+i)!=expected[i]) abort();
    for (unsigned i=0;i<4096;i++)
        if (PE_LoadU32(0x80146000u+4*i)!=(i==135?0xaa145000u:0xaabbccddu)) abort();
    for (unsigned i=0;i<64;i++) if (PE_LoadU8(0x1f800340u+i)!=(uint8_t)i) abort();
}
int main(void)
{
    run(0); run(1); PE_RamDestroy();
    puts("shadow native projection, packet, ordering and scratch controls pass");
    return 0;
}
