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
 if(argc!=2)return 2;int raw=atoi(argv[1]);uint32_t first=0x800a4000u,second=0x800a4100u;uint16_t p1=0,p2=0;int status=1,services=0;
 PE_Port_RunControlReset();PE_RamReset();PE_Sdk_ResetState();HostFB_Init();PE_GPU_Init();PE_GPU_WriteDPCR(0x33333b33u);
 PE_StoreU32(first,0x030a4100u);PE_StoreU32(first+4u,0x020000ffu);PE_StoreU32(first+8u,(20u<<16)|10u);PE_StoreU32(first+12u,(3u<<16)|4u);
 PE_StoreU32(second,0x03ffffffu);PE_StoreU32(second+4u,0x0200ff00u);PE_StoreU32(second+8u,(40u<<16)|50u);PE_StoreU32(second+12u,(3u<<16)|4u);
 if(!raw){status=func_80076B98(first,0);}else{status=PE_GPU_WriteGP1(0x04000002u)&&PE_GPU_WriteDMA2Register(0,first)&&PE_GPU_WriteDMA2Register(4,0)&&PE_GPU_WriteDMA2Register(8,0x01000401u);while(status&&PE_GPU_DMA2Pending()&&services<8){int completed=PE_GPU_ServiceDMA2Completion(PE_GPU_DMA2EventToken());services++;if(PE_Port_ShouldStop())status=0;else if(completed)break;}}
 PeGpuState g;PE_GPU_GetState(&g);PE_GPU_ReadVRAM(10,20,&p1);PE_GPU_ReadVRAM(50,40,&p2);
 printf("path=%d ret=%d stop=%d pixels=%04x,%04x fills=%llu gp0=%d pending=%d chcr=%08x madr=%08x services=%d\n",raw,status,PE_Port_GetStopReason(),p1,p2,(unsigned long long)g.fill_count,g.gp0_state,PE_GPU_DMA2Pending(),PE_GPU_ReadDMA2CHCR(),PE_GPU_ReadDMA2MADR(),services);return 0;
}
