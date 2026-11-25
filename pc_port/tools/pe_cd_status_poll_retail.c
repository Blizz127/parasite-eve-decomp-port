/* Test-only original-code model; callbacks and timing are scripted. */
#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);typedef void(*Store)(uint32_t,uint32_t,uint32_t);typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t mode,uint32_t dst,Load ld,Store st,Call call){
 uint32_t now=call(0x80073a44u,0xffffffffu,0,0,0,0);st(0x800a3478u,now+0x3c0u,4);st(0x800a347cu,0,4);st(0x800a3480u,0x80011ba0u,4);
 for(;;){
  now=call(0x80073a44u,0xffffffffu,0,0,0,0);uint32_t deadline=ld(0x800a3478u,4);int timed=(int32_t)deadline<(int32_t)now;
  if(!timed){uint32_t prev=ld(0x800a347cu,4);st(0x800a347cu,prev+1u,4);timed=(int32_t)prev>0x3c0000;}
  if(timed){
   (void)call(0x80073c5cu,0x80011b18u,0,0,0,0);
   uint32_t state=ld(0x8009b294u,1),index2=ld(0x8009b295u,1),msg=ld(0x800a3480u,4);
   uint32_t v=ld(0x8009b05cu+4u*index2,4),idx=ld(0x8009afd5u,1),a2=ld(0x8009afdcu+4u*idx,4),a3=ld(0x8009b05cu+4u*state,4);
   (void)call(0x80071a74u,0x80011b28u,msg,a2,a3,v);(void)call(0x8007b9ecu,0,0,0,0,0);return 0xffffffffu;
  }
  if(call(0x80073de8u,0,0,0,0,0)){
   uint32_t p=ld(0x8009b27cu,4),saved=ld(p,1)&3u,bits;
   while((bits=call(0x8007aab4u,0,0,0,0,0))!=0){
    if(bits&4u){uint32_t cb=ld(0x8009afb8u,4);if(cb){uint32_t status=ld(0x8009b295u,1);(void)call(cb,status,0x800a3468u,0,0,0);}}
    if(bits&2u){uint32_t cb=ld(0x8009afb4u,4);if(cb){uint32_t status=ld(0x8009b294u,1);(void)call(cb,status,0x800a3460u,0,0,0);}}
   }
   p=ld(0x8009b27cu,4);st(p,saved,1);
  }
  uint32_t state=ld(0x8009b294u,1);
  if(state==2u||state==5u){st(0x8009b294u,2,1);if(dst)for(unsigned i=0;i<8;i++)st(dst+i,ld(0x800a3460u+i,1),1);return state;}
  if(mode)return 0;
 }
}
