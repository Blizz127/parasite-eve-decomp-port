#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_gpu.h"
#include "host_framebuffer.h"
#include "pe_sdk.h"
#include "psx_compat.h"
void Trace_Direct(const char*s){(void)s;}
int main(int argc,char**argv){
 if(argc!=2)return 2;int raw=atoi(argv[1]);uint32_t node=0x800a4000u;uint16_t px=0;int status=1,services=0;
 PE_Port_RunControlReset();PE_RamReset();PE_Sdk_ResetState();HostFB_Init();PE_GPU_Init();PE_GPU_WriteDPCR(0x33333b33u);
 PE_StoreU32(node,0x03ffffffu);PE_StoreU32(node+4u,0x020000ffu);PE_StoreU32(node+8u,(20u<<16)|10u);PE_StoreU32(node+12u,(3u<<16)|4u);
 if(!raw){status=func_80076B98(node,0);} else {
  status=PE_GPU_WriteGP1(0x04000002u)&&PE_GPU_WriteDMA2Register(0,node)&&PE_GPU_WriteDMA2Register(4,0)&&PE_GPU_WriteDMA2Register(8,0x01000401u);
  while(status && PE_GPU_DMA2Pending() && services<8){status=PE_GPU_ServiceDMA2Completion(PE_GPU_DMA2EventToken());services++;}
 }
 PeGpuState g;PE_GPU_GetState(&g);PE_GPU_ReadVRAM(10,20,&px);
 printf("path=%d ret=%d stop=%d pixel=%04x fill=%llu gp0=%d pending=%d chcr=%08x madr=%08x bcr=%08x services=%d\n",raw,status,PE_Port_GetStopReason(),px,(unsigned long long)g.fill_count,g.gp0_state,PE_GPU_DMA2Pending(),PE_GPU_ReadDMA2CHCR(),PE_GPU_ReadDMA2MADR(),PE_GPU_ReadDMA2BCR(),services);
 return 0;
}
