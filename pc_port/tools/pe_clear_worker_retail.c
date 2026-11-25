/* Test-only finite ClearImage packet model; GPU calls are supplied by the checker. */
#include <stdint.h>
typedef uint32_t (*load_t)(uint32_t,uint32_t);
typedef void (*store_t)(uint32_t,uint32_t,uint32_t);
typedef uint32_t (*call_t)(uint32_t,uint32_t,uint32_t);
int draft(uint32_t rect,uint32_t color,load_t ld,store_t st,call_t call){
 int32_t width=(int16_t)ld(rect+4,2),limw=(int16_t)ld(0x80095750u,2),height=(int16_t)ld(rect+6,2);
 width=width<0?0:width>=limw?limw-1:width;st(rect+4,(uint16_t)width,2);
 int32_t limh=(int16_t)ld(0x80095752u,2);uint32_t x=ld(rect,2);
 height=height<0?0:height>=limh?limh-1:height;st(rect+6,(uint16_t)height,2);
 int general=(x&63u)!=0 || (ld(rect+4,2)&63u)!=0;
 uint32_t status;
 if(!general){
  st(0x800a3300u,0x05ffffffu,4);st(0x800a3304u,0xe6000000u,4);
  status=ld(ld(0x80095854u,4),4);st(0x800a330cu,0x02000000u|(color&0xffffffu),4);st(0x800a3308u,0xe1000000u|((status&0x7ffu)|((color>>21)&0x400u)),4);
  st(0x800a3310u,ld(rect,4),4);st(0x800a3314u,ld(rect+4,4),4);
  (void)call(0x80076b98u,0x800a3300u,0x02000000u);
 } else {
  st(0x800a3300u,0x080a3328u,4);st(0x800a3310u,0xe6000000u,4);
  uint32_t ptr=ld(0x80095854u,4);st(0x800a3304u,0xe3000000u,4);st(0x800a3308u,0xe4ffffffu,4);st(0x800a330cu,0xe5000000u,4);
  status=ld(ptr,4);st(0x800a3318u,0x60000000u|(color&0xffffffu),4);st(0x800a3314u,0xe1000000u|((status&0x7ffu)|((color>>21)&0x400u)),4);
  st(0x800a331cu,ld(rect,4),4);uint32_t wh=ld(rect+4,4);st(0x800a3328u,0x03ffffffu,4);st(0x800a3320u,wh,4);
  st(0x800a332cu,0xe3000000u|(call(0x80076be0u,3,0)&0xfffffu),4);
  st(0x800a3330u,0xe4000000u|(call(0x80076be0u,4,0)&0xfffffu),4);
  st(0x800a3334u,0xe5000000u|(call(0x80076be0u,5,0)&0x3fffffu),4);
  (void)call(0x80076b98u,0x800a3300u,0x800a3300u);
 }
 return 0;
}
