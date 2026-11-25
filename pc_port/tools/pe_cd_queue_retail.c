/* Independent, test-only scratch draft from original execution observations. */
#include <stdint.h>
typedef uint32_t (*load_fn)(uint32_t,uint32_t);
typedef void (*store_fn)(uint32_t,uint32_t,uint32_t);
typedef uint32_t (*call_fn)(uint32_t,uint32_t,uint32_t);
static uint32_t enqueue(uint32_t cmd,uint32_t p,uint32_t r,uint32_t cb,load_fn ld,store_fn st,call_fn call)
{
 if((int32_t)ld(0x800A3608u,4)>=8)return 0;
 uint32_t serial=ld(0x8009B53Cu,4)+1u;st(0x8009B53Cu,4,serial);
 if(!serial){serial=1;st(0x8009B53Cu,4,serial);}
 uint32_t rec=call(0x8007E6B0u,0,0);
 st(rec,4,serial);st(rec+4,1,cmd&255u);if(p)call(0x80080950u,rec+5,p);
 st(rec+12,4,p);st(rec+16,4,r);st(rec+20,4,cb);
 st(0x800A3608u,4,ld(0x800A3608u,4)+1u);
 if(call(0x8007FBF0u,0,0)==1u){
  uint32_t idx=ld(0x800A3604u,4);
  if(ld(0x800A3540u+24u*idx,4)==serial)call(0x8007E8F4u,0,0);
 }
 return serial;
}
uint32_t draft(uint32_t cmd,uint32_t p,uint32_t r,uint32_t cb,load_fn ld,store_fn st,call_fn call)
{
 cmd&=255u;
 if(ld(0x8009B4BCu+4u*cmd,4)&&p&&!enqueue(2,p,0,0,ld,st,call))return 0;
 return enqueue(cmd,p,r,cb,ld,st,call);
}
