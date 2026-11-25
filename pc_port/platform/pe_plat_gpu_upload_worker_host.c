/* Native GPU upload worker; retain existing readiness and span guards. */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_gpu.h"
#include "game_port.h"
#include "psx_compat.h"
extern int pe_plat_gpu_upload_span_existing(pe_addr_t source,uint32_t words);
extern int pe_plat_gpu_upload_wait_existing(void);
static int upload(uint32_t xy,uint32_t wh,uint32_t guest_rect,int writeback,uint32_t source){
 int32_t width=(int16_t)wh,height=(int16_t)(wh>>16),limitw=(int16_t)PE_LoadU16(0x80095750u),limith=(int16_t)PE_LoadU16(0x80095752u);
 if(width<0)width=0;else if(width>limitw)width=limitw;
 if(height<0)height=0;else if(height>limith)height=limith;
 if(writeback){PE_StoreU16(guest_rect+4,(uint16_t)width);PE_StoreU16(guest_rect+6,(uint16_t)height);}
 int32_t pixels=(int16_t)width*(int16_t)height;int32_t words=(pixels+1)/2;if(words<=0)return -1;
 uint32_t count=(uint32_t)words&15u,blocks=(uint32_t)words>>4;
 if(!pe_plat_gpu_upload_span_existing(source,(uint32_t)words))return -1;
 if(!pe_plat_gpu_upload_wait_existing())return -1;
 if(!PE_GPU_CanBeginImageLoad(blocks!=0u))return -1;
 if(!PE_GPU_WriteGP1(0x04000000u))return -1;
 if(!PE_GPU_WriteGP0(0x01000000u))return -1;
 if(!PE_GPU_WriteGP0(0xa0000000u))return -1;
 if(!PE_GPU_WriteGP0(xy))return -1;
 if(!PE_GPU_WriteGP0(((uint32_t)(uint16_t)height<<16)|(uint16_t)width))return -1;
 for(uint32_t i=0;i<count;i++)if(!PE_GPU_WriteGP0(PE_LoadU32(source+4u*i)))return -1;
 if(blocks){if(!PE_GPU_WriteGP1(0x04000002u))return -1;if(!PE_GPU_DMA2Issue(source+4u*count,(blocks<<16)|16u,0x01000201u))return -1;}
 return 0;
}
int pe_plat_gpu_upload_worker_host(uint32_t rect,uint32_t source){
 (void)func_800773D0();if(!PE_RangeIsRam(rect,8u))return -1;
 uint32_t xy=PE_LoadU32(rect),wh=PE_LoadU32(rect+4u);return upload(xy,wh,rect,1,source);
}
int pe_plat_gpu_upload_inline_host(uint32_t xy,uint32_t wh,uint32_t source){
 (void)func_800773D0();return upload(xy,wh,0,0,source);
}
