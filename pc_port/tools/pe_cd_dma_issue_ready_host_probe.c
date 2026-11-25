#include <stdint.h>
#include <stdio.h>
#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_cdreg.h"
#include "pe_gpu.h"
#include "host_framebuffer.h"
#include "pe_sdk.h"
void Trace_Direct(const char *s){(void)s;}
int main(void){unsigned d,p,dst,blocks,words,control,intr;unsigned i=0;
 while(scanf("%x %x %x %x %x %x %x",&d,&p,&dst,&blocks,&words,&control,&intr)==7){
  PE_Port_RunControlReset();PE_RamReset();PE_Sdk_ResetState();PE_CdReg_Reset();HostFB_Init();PE_GPU_Init();
  PE_StoreU32(0x8009b348u,0x80130000u);PE_StoreU32(0x80130000u,d);PE_StoreU32(0x8009b344u,0x80130004u);PE_StoreU32(0x80130004u,p);
  PE_StoreU32(0x8009b32cu,0x80130008u);PE_StoreU8(0x80130008u,0x40u);PE_CdStoreU32(PE_CDREG_DMA3+8u,0u);
  func_8007CEAC(3u,dst,blocks,words,control,intr,0u);
  printf("%u %d %08x %08x %08x %08x %08x\n",i++,PE_Port_GetStopReason(),PE_LoadU32(0x80130000u),PE_LoadU32(0x80130004u),PE_CdLoadU32(PE_CDREG_DMA3),PE_CdLoadU32(PE_CDREG_DMA3+4u),PE_CdLoadU32(PE_CDREG_DMA3+8u));
 }
 return 0;}
