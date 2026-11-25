/* Test-only oracle model; unknown jump targets are outside its verified domain. */
#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);typedef void(*Store)(uint32_t,uint32_t,uint32_t);typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t prefix_model(uint8_t *,uint32_t *,uint32_t *,Load,Store,Call);
uint32_t case_model(uint32_t,uint32_t,uint32_t,const uint8_t *,Load,Store);
uint32_t draft(Load ld,Store st,Call call){uint8_t packet[8]={0};uint32_t count=0,status=0;uint32_t irq=prefix_model(packet,&count,&status,ld,st,call);if(irq==8u)return 0;
 if(irq==0u || irq>5u){(void)call(0x80011b6cu,0,0,0);(void)call(0x80011b80u,irq,0,0);return 0;}
 uint32_t target=ld(0x80011b8cu+4u*(irq-1u),4);const uint32_t cases[]={0x8007ae5cu,0x8007ae10u,0x8007ad10u,0x8007aedcu,0x8007af5cu};
 for(uint32_t i=0;i<5u;i++)if(target==cases[i])return case_model(i+1u,count,status,packet,ld,st);return 0;}
