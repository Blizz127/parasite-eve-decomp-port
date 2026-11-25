/* Synthetic tray -> native completion writer -> query chain. No media/devices. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_drive_tray.h"
#include "pe_guest_ram.h"
#include "pe_mmio.h"
#include "pe_plat/cd.h"

int PE_MMIO_InShadowBlock(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void *PE_MMIO_Translate(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void PE_MMIO_Store(pe_addr_t a,size_t n,uint32_t v) { (void)a;(void)n;(void)v;abort(); }
static unsigned polls,escapes;
static int stopped;
void PE_Port_EscapePoint(void) { escapes++; }
int PE_Port_ShouldStop(void) { return stopped; }
int __wrap_pe_plat_cd_poll_sync(uint32_t result) { if(result) abort(); polls++; return 0; }
void HostFB_DevicePoll(void) { abort(); }

static void run(uint32_t slot,uint32_t previous,uint32_t sequence,uint32_t next_slot)
{
    PE_RamReset(); polls=escapes=0; stopped=0;
    PE_StoreU32(0x800a3690u,slot); PE_StoreU32(0x8009b53cu,previous);
    PE_Fill(0x80140000u,8,0x55);
    if(PE_Tray_HostStop()!=sequence || PE_LoadU32(0x8009b53cu)!=sequence ||
       PE_LoadU32(0x800a3690u)!=next_slot) abort();
    pe_addr_t entry=0x800a3610u+16u*slot;
    if(PE_LoadU32(entry)!=sequence || PE_LoadU8(entry+4)!=2) abort();
    for(unsigned i=0;i<8;i++) if(PE_LoadU8(entry+5+i)!=0) abort();
    if(pe_plat_cd_response_query_host(sequence,0x80140000u)!=2 || polls!=1 || escapes!=1) abort();
    for(unsigned i=0;i<8;i++) if(PE_LoadU8(0x80140000u+i)!=0) abort();
    stopped=1; PE_Fill(0x80140000u,8,0x55);
    if(pe_plat_cd_response_query_host(sequence,0x80140000u)!=0 || polls!=2 || escapes!=2) abort();
    for(unsigned i=0;i<8;i++) if(PE_LoadU8(0x80140000u+i)!=0x55) abort();
    pe_addr_t frame=PE_GuestFrame_Enter(8);
    if(frame!=PE_GUEST_FRAME_HI-8) abort();
    PE_GuestFrame_Leave(frame,8);
}
int main(void)
{
    run(0,0,1,1); run(7,UINT32_MAX,1,0);
    PE_RamDestroy(); puts("tray/native writer/response query: normal, ring wrap and stop pass");
    return 0;
}
