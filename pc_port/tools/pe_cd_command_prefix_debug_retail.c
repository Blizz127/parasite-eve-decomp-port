/* Test-only original-code model; callbacks and timing are scripted. */
#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);typedef void(*Store)(uint32_t,uint32_t,uint32_t);typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t);
/* Test-only command prefix, debug disabled; 0x100 denotes entering sync wait. */
uint32_t draft(uint32_t cmd,uint32_t input,uint32_t mode,Load ld,Store st,Call call){
 uint32_t debug=ld(0x8009afc0u,4),id=cmd&255u; if((int32_t)debug>=2) (void)call(0x80071a74u,0x80011bb4u,ld(0x8009afdcu+id*4u,4));
 if(ld(0x8009b1fcu+id*4u,4) && !input){debug=ld(0x8009afc0u,4);if((int32_t)debug>0)(void)call(0x80071a74u,0x80011bbcu,ld(0x8009afdcu+id*4u,4));return 0xfffffffeu;}
 (void)call(0x8007b010u,0,0);
 if(id==2u){for(unsigned i=0;i<4u;i++)st(0x8009afd0u+i,ld(input+i,1),1);}
 if(id==14u)st(0x8009afd4u,ld(input,1),1);
 st(0x8009b294u,0,1);if(ld(0x8009b0fcu+id*4u,4))st(0x8009b295u,0,1);
 uint32_t p=ld(0x8009b27cu,4);st(p,0,1);uint32_t count=ld(0x8009b1fcu+id*4u,4),i=0;
 if((int32_t)count>0)do{p=ld(0x8009b284u,4);uint8_t v=ld(input+i,1);st(p,v,1);i++;count=ld(0x8009b1fcu+id*4u,4);}while((int32_t)i<(int32_t)count);
 p=ld(0x8009b280u,4);st(0x8009afd5u,id,1);st(p,id,1);return mode?0u:0x100u;
}
