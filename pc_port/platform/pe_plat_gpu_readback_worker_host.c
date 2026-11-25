/* Native GPU readback worker; retain the existing readiness boundary. */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_gpu.h"
#include "game_port.h"
#include "psx_compat.h"
extern int pe_plat_gpu_readback_wait_existing(uint32_t ready);
static int readback(uint32_t xy,uint32_t wh,uint32_t guest_rect,int writeback,uint32_t destination){
 int32_t width=(int16_t)wh,height=(int16_t)(wh>>16),limitw=(int16_t)PE_LoadU16(0x80095750u),limith=(int16_t)PE_LoadU16(0x80095752u);
 if(width<0)width=0;else if(width>limitw)width=limitw;
 if(height<0)height=0;else if(height>limith)height=limith;
 if(writeback){PE_StoreU16(guest_rect+4,(uint16_t)width);PE_StoreU16(guest_rect+6,(uint16_t)height);}
 int32_t pixels=(int16_t)width*(int16_t)height;int32_t words=(pixels+1)/2;if(words<=0)return -1;
 uint32_t count=(uint32_t)words&15u,blocks=(uint32_t)words>>4;
 if(!destination || (destination&3u) || (uint32_t)words>0x3fffffffu || !PE_RangeIsRam(destination,(uint32_t)words*4u))return -1;
 if(!pe_plat_gpu_readback_wait_existing(0x04000000u))return -1;
 if(!PE_GPU_CanBeginImageLoad(blocks!=0u))return -1;
 if(!PE_GPU_WriteGP1(0x04000000u))return -1;
 if(!PE_GPU_WriteGP0(0x01000000u))return -1;
 if(!PE_GPU_WriteGP0(0xc0000000u))return -1;
 if(!PE_GPU_WriteGP0(xy))return -1;
 if(!PE_GPU_WriteGP0(((uint32_t)(uint16_t)height<<16)|(uint16_t)width))return -1;
 if(!pe_plat_gpu_readback_wait_existing(0x08000000u))return -1;
 for(uint32_t i=0;i<count;i++){uint32_t value;if(!PE_GPU_ReadGP0(&value))return -1;PE_StoreU32(destination+4u*i,value);}
 if(blocks){if(!PE_GPU_WriteGP1(0x04000003u))return -1;if(!PE_GPU_DMA2Issue(destination+4u*count,(blocks<<16)|16u,0x01000200u))return -1;}
 return 0;
}
int pe_plat_gpu_readback_worker_host(uint32_t rect,uint32_t destination){
 (void)func_800773D0();if(!PE_RangeIsRam(rect,8u))return -1;
 uint32_t xy=PE_LoadU32(rect),wh=PE_LoadU32(rect+4u);return readback(xy,wh,rect,1,destination);
}
int pe_plat_gpu_readback_inline_host(uint32_t xy,uint32_t wh,uint32_t destination){
 (void)func_800773D0();return readback(xy,wh,0,0,destination);
}
