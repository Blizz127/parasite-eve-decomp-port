#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);typedef void(*Store)(uint32_t,uint32_t,uint32_t);typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t draft(Load ld,Store st,Call call){
 (void)call(0x80073c5cu,0x80011c08u,0,0,0);(void)call(0x80071a74u,0x80011c14u,0x8009b298u,0,0);
 st(0x8009afd5u,0,1);st(0x8009afd4u,0,1);st(0x8009afb8u,0,4);st(0x8009afb4u,0,4);st(0x8009afc8u,0,4);st(0x8009afc4u,0,4);
 (void)call(0x80073c94u,0,0,0,0);(void)call(0x80073cc4u,2,0x8007c13cu,0,0);
 uint32_t p=ld(0x8009b27cu,4);st(p,1,1);p=ld(0x8009b288u,4);
 while(ld(p,1)&7u){p=ld(0x8009b27cu,4);st(p,1,1);p=ld(0x8009b288u,4);st(p,7,1);p=ld(0x8009b284u,4);st(p,7,1);p=ld(0x8009b288u,4);}
 st(0x8009b296u,0,1);uint32_t v=ld(0x8009b296u,1);st(0x8009b295u,v,1);p=ld(0x8009b27cu,4);st(0x8009b294u,2,1);st(p,0,1);p=ld(0x8009b288u,4);st(p,0,1);p=ld(0x8009b28cu,4);st(p,0x1325,4);
 (void)call(0x8007b558u,1,0,0,0);if(ld(0x8009afc4u,4)&0x10u)(void)call(0x8007b558u,1,0,0,0);
 if(call(0x8007b558u,10,0,0,0))return 0xffffffffu;
 if(call(0x8007b558u,12,0,0,0))return 0xffffffffu;
 return call(0x8007b010u,0,0,0,0)==2?0:0xffffffffu;
}
