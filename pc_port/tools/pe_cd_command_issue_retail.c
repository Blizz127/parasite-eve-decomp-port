/* Test-only original-code model; callbacks and timing are scripted. */
#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);typedef void(*Store)(uint32_t,uint32_t,uint32_t);typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
/* Test-only command prefix, debug disabled; 0x100 denotes entering sync wait. */
static uint32_t prefix(uint32_t cmd,uint32_t input,uint32_t mode,Load ld,Store st,Call call){
 uint32_t debug=ld(0x8009afc0u,4),id=cmd&255u; if((int32_t)debug>=2) (void)call(0x80071a74u,0x80011bb4u,ld(0x8009afdcu+id*4u,4),0,0,0);
 if(ld(0x8009b1fcu+id*4u,4) && !input){debug=ld(0x8009afc0u,4);if((int32_t)debug>0)(void)call(0x80071a74u,0x80011bbcu,ld(0x8009afdcu+id*4u,4),0,0,0);return 0xfffffffeu;}
 (void)call(0x8007b010u,0,0,0,0,0);
 if(id==2u){for(unsigned i=0;i<4u;i++)st(0x8009afd0u+i,ld(input+i,1),1);}
 if(id==14u)st(0x8009afd4u,ld(input,1),1);
 st(0x8009b294u,0,1);if(ld(0x8009b0fcu+id*4u,4))st(0x8009b295u,0,1);
 uint32_t p=ld(0x8009b27cu,4);st(p,0,1);uint32_t count=ld(0x8009b1fcu+id*4u,4),i=0;
 if((int32_t)count>0)do{p=ld(0x8009b284u,4);uint8_t v=ld(input+i,1);st(p,v,1);i++;count=ld(0x8009b1fcu+id*4u,4);}while((int32_t)i<(int32_t)count);
 p=ld(0x8009b280u,4);st(0x8009afd5u,id,1);st(p,id,1);return mode?0u:0x100u;
}

static uint32_t waitpart(uint32_t dst,Load ld,Store st,Call call){
 uint32_t now=call(0x80073a44u,0xffffffffu,0,0,0,0);st(0x800a3478u,now+0x3c0u,4);st(0x800a347cu,0,4);uint32_t state=ld(0x8009b294u,1);st(0x800a3480u,0x80011bccu,4);
 while(!state){
  now=call(0x80073a44u,0xffffffffu,0,0,0,0);uint32_t deadline=ld(0x800a3478u,4);int timed=(int32_t)deadline<(int32_t)now;
  if(!timed){uint32_t prev=ld(0x800a347cu,4);st(0x800a347cu,prev+1u,4);timed=(int32_t)prev>0x3c0000;}
  if(timed){
   (void)call(0x80073c5cu,0x80011b18u,0,0,0,0);
   uint32_t a0=ld(0x8009b294u,1),index2=ld(0x8009b295u,1),msg=ld(0x800a3480u,4);
   uint32_t v=ld(0x8009b05cu+4u*index2,4),idx=ld(0x8009afd5u,1),a2=ld(0x8009afdcu+4u*idx,4),a3=ld(0x8009b05cu+4u*a0,4);
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
  state=ld(0x8009b294u,1);
 }
 if(dst)for(unsigned i=0;i<8;i++)st(dst+i,ld(0x800a3460u+i,1),1);
 state=ld(0x8009b294u,1);return state==5u?0xffffffffu:0;
}

uint32_t draft(uint32_t cmd,uint32_t input,uint32_t dst,uint32_t mode,Load ld,Store st,Call call){uint32_t x=prefix(cmd,input,mode,ld,st,call);return x==0x100u?waitpart(dst,ld,st,call):x;}
