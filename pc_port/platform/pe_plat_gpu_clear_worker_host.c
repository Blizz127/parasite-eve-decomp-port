/* Native clear worker; guest RECT and queued host words use separate views. */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_gpu.h"
#include "game_port.h"
#include "psx_compat.h"
typedef struct { uint32_t rect; uint32_t *words; } RectView;
static uint32_t read16(const RectView *v,uint32_t offset){
 if(v->words)return (v->words[offset>>2] >> ((offset&2u)*8u)) & 0xffffu;
 return PE_LoadU16(v->rect+offset);
}
static uint32_t read32(const RectView *v,uint32_t offset){return v->words?v->words[offset>>2]:PE_LoadU32(v->rect+offset);}
static void write16(const RectView *v,uint32_t offset,uint32_t value){
 if(v->words){unsigned shift=(offset&2u)*8u;v->words[offset>>2]=(v->words[offset>>2]&~(0xffffu<<shift))|((value&0xffffu)<<shift);}
 else PE_StoreU16(v->rect+offset,(uint16_t)value);
}
static int run_clear(const RectView *r,uint32_t color){
 int32_t width=(int16_t)read16(r,4), limw=(int16_t)PE_LoadU16(0x80095750u), height=(int16_t)read16(r,6);
 width=width<0?0:width>=limw?limw-1:width;write16(r,4,(uint16_t)width);
 int32_t limh=(int16_t)PE_LoadU16(0x80095752u);uint32_t x=read16(r,0);
 height=height<0?0:height>=limh?limh-1:height;write16(r,6,(uint16_t)height);
 int general=(x&63u)!=0 || (read16(r,4)&63u)!=0;uint32_t status;
 if(!general){
  PE_StoreU32(0x800a3300u,0x05ffffffu);PE_StoreU32(0x800a3304u,0xe6000000u);
  status=PE_GPU_ReadStatus();PE_StoreU32(0x800a330cu,0x02000000u|(color&0xffffffu));PE_StoreU32(0x800a3308u,0xe1000000u|((status&0x7ffu)|((color>>21)&0x400u)));
  PE_StoreU32(0x800a3310u,read32(r,0));PE_StoreU32(0x800a3314u,read32(r,4));
 }else{
  PE_StoreU32(0x800a3300u,0x080a3328u);PE_StoreU32(0x800a3310u,0xe6000000u);
  PE_StoreU32(0x800a3304u,0xe3000000u);PE_StoreU32(0x800a3308u,0xe4ffffffu);PE_StoreU32(0x800a330cu,0xe5000000u);
  status=PE_GPU_ReadStatus();PE_StoreU32(0x800a3318u,0x60000000u|(color&0xffffffu));PE_StoreU32(0x800a3314u,0xe1000000u|((status&0x7ffu)|((color>>21)&0x400u)));
  PE_StoreU32(0x800a331cu,read32(r,0));uint32_t wh=read32(r,4);PE_StoreU32(0x800a3328u,0x03ffffffu);PE_StoreU32(0x800a3320u,wh);
  PeGpuState gpu;PE_GPU_GetState(&gpu);PE_StoreU32(0x800a332cu,0xe3000000u|(gpu.drawing_area_top_left&0xfffffu));
  PE_StoreU32(0x800a3330u,0xe4000000u|(gpu.drawing_area_bottom_right&0xfffffu));PE_StoreU32(0x800a3334u,0xe5000000u|(gpu.drawing_offset&0x3fffffu));
 }
 (void)func_80076B98(0x800a3300u,0u);return 0;
}
int pe_plat_gpu_clear_worker_host(uint32_t rect,uint32_t color){RectView r={rect,0};return run_clear(&r,color);}
int pe_plat_gpu_clear_inline_host(uint32_t words[2],uint32_t color){RectView r={0,words};return run_clear(&r,color);}
