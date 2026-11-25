/* Synthetic tray stop integration with real native guest-frame backend. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_drive_tray.h"
#include "pe_guest_ram.h"
#include "pe_mmio.h"

int PE_MMIO_InShadowBlock(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void *PE_MMIO_Translate(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
uint32_t PE_MMIO_Load(pe_addr_t a,size_t n) { (void)a;(void)n;abort(); }
void PE_MMIO_Store(pe_addr_t a,size_t n,uint32_t v) { (void)a;(void)n;(void)v;abort(); }
static unsigned calls;
static uint32_t want_seq;
void pe_plat_ds_text_5f4(uint32_t sequence, uint32_t status, uint32_t response)
{
    if(sequence!=want_seq || status!=2 || response!=PE_GUEST_FRAME_HI-8) abort();
    for(unsigned i=0;i<8;i++) if(PE_LoadU8(response+i)!=0) abort();
    pe_addr_t nested=PE_GuestFrame_Enter(9);
    if(nested!=response-16) abort();
    PE_GuestFrame_Leave(nested,9);
    calls++;
}
static void run(uint32_t old,uint32_t next)
{
    PE_StoreU32(0x8009b53cu,old);
    want_seq=next;
    if(PE_Tray_HostStop()!=next || PE_LoadU32(0x8009b53cu)!=next) abort();
    pe_addr_t f=PE_GuestFrame_Enter(8);
    if(f!=PE_GUEST_FRAME_HI-8) abort();
    PE_GuestFrame_Leave(f,8);
}
int main(void)
{
    PE_RamInit();
    run(0,1); run(41,42); run(UINT32_MAX,1);
    if(calls!=3) abort();
    PE_RamDestroy();
    puts("synthetic tray stop sequence, nested frame and restoration pass");
    return 0;
}
