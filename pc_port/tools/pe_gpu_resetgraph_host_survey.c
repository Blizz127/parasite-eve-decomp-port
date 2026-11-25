#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_gpu.h"
#include "host_framebuffer.h"
#include "pe_sdk.h"
#include "psx_compat.h"
void Trace_Direct(const char *s){(void)s;}
int main(int argc,char **argv){
 if(argc!=4)return 2;
 int mode=atoi(argv[1]); unsigned seed=strtoul(argv[2],0,0),init=strtoul(argv[3],0,0);
 PE_Port_RunControlReset();PE_RamReset();PE_Sdk_ResetState();HostFB_Init();PE_GPU_Init();
 for(unsigned i=0;i<0x200;i++)PE_StoreU8(0x80095700u+i,(uint8_t)(seed+i*7u));
 PE_StoreU8(0x8009574cu,(uint8_t)init);PE_GPU_WriteDPCR(0x33333333u);
 uint32_t dpcr_before=PE_GPU_ReadDPCR();int rv=func_80074A44(mode);PeGpuState gpu;PE_GPU_GetState(&gpu);
 uint64_t h=14695981039346656037ull;for(unsigned i=0;i<0x200;i++){h^=PE_LoadU8(0x80095700u+i);h*=1099511628211ull;}
 printf("mode=%d seed=%u init=%u rv=%d stop=%d epoch=%u dpcr=%08x>%08x status=%08x limits=%u,%u initbyte=%u regionhash=%016llx\n",mode,seed,init,rv,PE_Port_GetStopReason(),PE_Port_StopEpoch(),dpcr_before,PE_GPU_ReadDPCR(),gpu.status,PE_LoadU16(0x80095750u),PE_LoadU16(0x80095752u),PE_LoadU8(0x8009574cu),(unsigned long long)h);
 return 0;
}
