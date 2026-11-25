#include <stdint.h>
typedef uint32_t (*Load)(uint32_t,uint32_t);
typedef void (*Store)(uint32_t,uint32_t,uint32_t);
typedef uint32_t (*Call)(uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t out,uint32_t env,Load load,Store store,Call call){
 uint32_t a=(uint32_t)(int32_t)(int16_t)load(env,0x102),b=(uint32_t)(int32_t)(int16_t)load(env+2,0x102);
 uint32_t v=call(0x80076170u,a,b,0);store(out+4,v,4);
 a=load(env+4,2);v=load(env,2);b=load(env+2,2);a=(uint32_t)(int32_t)(int16_t)(a+v-1u);v=load(env+6,2);b=(uint32_t)(int32_t)(int16_t)(b+v-1u);
 v=call(0x80076208u,a,b,0);store(out+8,v,4);
 a=(uint32_t)(int32_t)(int16_t)load(env+8,0x102);b=(uint32_t)(int32_t)(int16_t)load(env+10,0x102);v=call(0x800762a0u,a,b,0);store(out+12,v,4);
 a=load(env+0x17,1);b=load(env+0x16,1);uint32_t c=load(env+0x14,2);v=call(0x80076150u,a,b,c);store(out+16,v,4);
 v=call(0x800762bcu,env+12,0,0);store(out+20,v,4);store(out+24,0xe6000000u,4);
 uint32_t n=7;
 if(load(env+0x18,1)){
  uint16_t x=load(env,2),y=load(env+2,2),width=load(env+4,2),height=load(env+6,2);
  if((int16_t)width<0)width=0;else {int32_t limit=(int16_t)load(0x80095750u,0x102)-1;uint16_t lim=load(0x80095750u,2);if(limit<(int16_t)width)width=lim-1u;}
  if((int16_t)height<0)height=0;else {int32_t limit=(int16_t)load(0x80095752u,0x102)-1;uint16_t lim=load(0x80095752u,2);if(limit<(int16_t)height)height=lim-1u;}
  uint32_t op;
  if((x&63u)||(width&63u)){
   x=(uint16_t)(x-load(env+8,2));y=(uint16_t)(y-load(env+10,2));op=0x60000000u;
  }else op=0x02000000u;
  uint32_t blue=load(env+0x1b,1),green=load(env+0x1a,1),red=load(env+0x19,1);
  store(out+4*n++,op|(blue<<16)|(green<<8)|red,4);
  store(out+4*n++,(uint32_t)x|((uint32_t)y<<16),4);
  store(out+4*n++,(uint32_t)width|((uint32_t)height<<16),4);
 }
 store(out+3,n-1u,1);return n-1u;
}
