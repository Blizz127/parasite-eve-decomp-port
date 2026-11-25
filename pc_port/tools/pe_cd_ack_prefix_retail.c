/* Test-only oracle model; unknown jump targets are outside its verified domain. */
#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);typedef void(*Store)(uint32_t,uint32_t,uint32_t);typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t draft(uint8_t *packet,uint32_t *count,uint32_t *status,Load ld,Store st,Call call){
 uint32_t p=ld(0x8009b27cu,4);st(p,1,1);p=ld(0x8009b288u,4);uint32_t irq=ld(p,1)&7u;
 if(!irq){*count=0;*status=0;return 8;}
 uint32_t q=ld(p,1)&7u;
 while(q!=irq){irq=ld(p,1)&7u;q=ld(p,1)&7u;}
 uint32_t n=0;
 while(n<8u){p=ld(0x8009b27cu,4);if(!(ld(p,1)&0x20u))break;p=ld(0x8009b280u,4);packet[n++]=ld(p,1);}
 for(uint32_t i=n;i<8;i++)packet[i]=0;
 p=ld(0x8009b27cu,4);st(p,1,1);p=ld(0x8009b288u,4);st(p,7,1);p=ld(0x8009b284u,4);st(p,7,1);
 uint32_t update=1;
 if(irq==3){uint32_t index=ld(0x8009afd5u,1);if(!ld(0x8009b17cu+index*4u,4))update=0;}
 *status=0;
 if(update){
  if(!(ld(0x8009afc4u,4)&0x10u) && (packet[0]&0x10u)){uint32_t v=ld(0x8009afccu,4);st(0x8009afccu,v+1,4);}
  *status=packet[0]&0x1du;st(0x8009afc4u,packet[0],4);st(0x8009afc8u,packet[1],4);
 }
 *count=n;
 if(irq==5 && (int32_t)ld(0x8009afc0u,4)>0){
  (void)call(0x80011b44u,0,0,0);
  if((int32_t)ld(0x8009afc0u,4)>0){uint32_t index=ld(0x8009afd5u,1);uint32_t a2=ld(0x8009afc4u,4);uint32_t a3=ld(0x8009afc8u,4);uint32_t a1=ld(0x8009afdcu+4u*index,4);(void)call(0x80011b50u,a1,a2,a3);}
 }
 return irq;
}
