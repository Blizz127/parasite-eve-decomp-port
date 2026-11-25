/* Test-only oracle model; unknown jump targets are outside its verified domain. */
#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);typedef void(*Store)(uint32_t,uint32_t,uint32_t);
static void copy(uint32_t dst,const uint8_t *packet,Store st){for(unsigned i=0;i<8;i++)st(dst+i,packet[i],1);}
uint32_t draft(uint32_t irq,uint32_t count,uint32_t status,const uint8_t *packet,Load ld,Store st){
 if(irq==1){if(status&&count==1)status=0;st(0x8009b295u,status?5:1,1);copy(0x800a3468u,packet,st);uint32_t p=ld(0x8009b27cu,4);st(p,0,1);p=ld(0x8009b288u,4);st(p,0,1);return 4;}
 if(irq==2){st(0x8009b294u,status?5:2,1);copy(0x800a3460u,packet,st);return 2;}
 if(irq==3){uint32_t state;if(status)state=5;else{uint32_t i=ld(0x8009afd5u,1);state=ld(0x8009b07cu+i*4u,4)?3:2;}st(0x8009b294u,state,1);copy(0x800a3460u,packet,st);return state==3?1:2;}
 if(irq==4){st(0x8009b296u,4,1);uint32_t v=ld(0x8009b296u,1);st(0x8009b295u,v,1);copy(0x800a3470u,packet,st);copy(0x800a3468u,packet,st);return 4;}
 if(irq==5){st(0x8009b295u,5,1);uint32_t v=ld(0x8009b295u,1);st(0x8009b294u,v,1);copy(0x800a3460u,packet,st);copy(0x800a3468u,packet,st);return 6;}
 return 0;
}
