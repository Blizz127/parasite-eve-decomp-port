/* Synthetic clean-room native CD command completion; no device or disc. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "game_port.h"
#include "pe_bootstrap.h"
#include "pe_guest_ram.h"
#include "pe_mmio.h"
#include "pe_sdk.h"

int PE_MMIO_InShadowBlock(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void *PE_MMIO_Translate(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void PE_MMIO_Store(pe_addr_t a,size_t n,uint32_t v) { (void)a;(void)n;(void)v;abort(); }
static int stopped, boundaries;
static uintptr_t seen_target;
int PE_Port_ShouldStop(void) { return stopped; }
void PE_Port_RequestStop(PEPortStopReason r)
{ if(r!=PE_PORT_STOP_UNRESOLVED_BOUNDARY) abort(); stopped=1; }
void Bootstrap_ReturnVoid4Indirect(const char *a,const char *b,uintptr_t target,
                                  uintptr_t c,uintptr_t d,uintptr_t e,uintptr_t f)
{ (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;seen_target=target;boundaries++; }
int PE_Cd_DispatchDataCallback(pe_addr_t a,uint32_t b,pe_addr_t c)
{ (void)a;(void)b;(void)c;abort(); }
void pe_plat_cd_command_complete(uint32_t status,pe_addr_t response);
static void init(uint8_t command)
{
    PE_RamReset(); stopped=boundaries=0;seen_target=0;
    PE_StoreU8(0x8009b558u,command);
}
int main(void)
{
    const pe_addr_t response=0x80140000u;
    init(8); PE_StoreU32(0x80011d04u+4u*8u,0x80080368u);
    pe_plat_cd_command_complete(2,response);
    if(stopped || PE_LoadU32(0x8009b574u)!=1 || PE_LoadU32(0x8009b578u)!=11) abort();
    init(14);PE_StoreU8(0x8009b559u,0x80);
    pe_plat_cd_command_complete(2,response);
    if(stopped || PE_LoadU32(0x8009b574u)!=2 || PE_LoadU32(0x8009b578u)!=15 ||
       PE_LoadU32(0x8009b594u)!=3 || PE_LoadU8(0x8009b581u)!=0x80) abort();
    init(14);PE_StoreU8(0x8009b559u,0x80);PE_StoreU8(0x8009b581u,0x80);
    pe_plat_cd_command_complete(2,response);
    if(stopped || PE_LoadU32(0x8009b574u)!=1 || PE_LoadU32(0x8009b578u)!=11 ||
       PE_LoadU8(0x8009b581u)!=0x80) abort();
    init(8);PE_StoreU32(0x80011d04u+4u*8u,0x81234567u);
    pe_plat_cd_command_complete(2,response);
    if(!stopped || boundaries!=1 || seen_target!=0x81234567u ||
       PE_LoadU32(0x8009b574u)!=0 || PE_LoadU32(0x8009b578u)!=0) abort();
    PE_RamDestroy();puts("synthetic CD stop, mode transitions and unknown target controls pass");
    return 0;
}
