#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_cdreg.h"
#include "pe_gpu.h"
#include "host_framebuffer.h"
#include "pe_sdk.h"
void Trace_Direct(const char *s){fprintf(stderr,"TRACE %s\n",s);}
int main(int argc,char **argv){
 unsigned ch=argc>1?strtoul(argv[1],0,0):3, ready=argc>2?strtoul(argv[2],0,0):1, busy=argc>3?strtoul(argv[3],0,0):0;
 PE_Port_RunControlReset();PE_RamReset();PE_Sdk_ResetState();PE_CdReg_Reset();HostFB_Init();PE_GPU_Init();
 PE_StoreU32(0x8009b348u,0x80130000u);PE_StoreU32(0x80130000u,0x428a1234u);
 PE_StoreU32(0x8009b344u,0x80130004u);PE_StoreU32(0x80130004u,0x12345678u);
 PE_StoreU32(0x8009b32cu,0x80130008u);PE_StoreU8(0x80130008u,ready?0x40u:0u);
 if(ch==3) PE_CdStoreU32(PE_CDREG_DMA3+8u,busy?0x01000000u:0u);
 func_8007CEAC(ch,0x80150000u,2u,8u,0x11000000u,1u,0u);
 printf("ch=%u ready=%u busy=%u stop=%d dicr=%08x dpcr=%08x madr=%08x bcr=%08x chcr=%08x\n",ch,ready,busy,PE_Port_GetStopReason(),PE_LoadU32(0x80130000u),PE_LoadU32(0x80130004u),ch==3?PE_CdLoadU32(PE_CDREG_DMA3):0u,ch==3?PE_CdLoadU32(PE_CDREG_DMA3+4u):0u,ch==3?PE_CdLoadU32(PE_CDREG_DMA3+8u):0u);
 uint32_t dicr=PE_LoadU32(0x80130000u), dpcr=PE_LoadU32(0x80130004u);
 if(ch!=3u) return !(PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY && dicr==0x428a1234u && dpcr==0x12345678u);
 if(busy) return !(PE_Port_GetStopReason()==PE_PORT_STOP_UNRESOLVED_BOUNDARY && dicr==0x428a1234u && dpcr==0x12345678u && PE_CdLoadU32(PE_CDREG_DMA3+8u)==0x01000000u);
 return !(PE_Port_GetStopReason()==(ready?PE_PORT_STOP_NONE:PE_PORT_STOP_UNRESOLVED_BOUNDARY) && dicr==0x428a1234u && dpcr==0x1234d678u && PE_CdLoadU32(PE_CDREG_DMA3)==0x80150000u && PE_CdLoadU32(PE_CDREG_DMA3+4u)==0x00020008u && PE_CdLoadU32(PE_CDREG_DMA3+8u)==(ready?0x11000000u:0u));
}
