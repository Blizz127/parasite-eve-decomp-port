/* Test-only draft inferred from original caller observations; not integrated. */
#include <stdint.h>
typedef uint32_t (*Load)(uint32_t,uint32_t);
typedef void (*Store)(uint32_t,uint32_t,uint32_t);
typedef uint32_t (*Call)(uint32_t,uint32_t,uint32_t);
static void issue(uint32_t command,uint32_t data,uint32_t marker,Load l,Store s,Call c){
 if((int32_t)l(0x8009B598u,4)<=0){s(0x8009B570u,4,marker);c(0x8007FCFCu,command,data);}
}
void draft(Load l,Store s,Call c){
 uint32_t timer=l(0x8009B598u,4);
 if((int32_t)timer>0){s(0x8009B598u,4,timer-1);if(timer==1){c(0x800800F4u,0,0);return;}}
 timer=l(0x8009B594u,4);if((int32_t)timer>0)s(0x8009B594u,4,timer-1);
 timer=l(0x8009B5A0u,4);if((int32_t)timer>0)s(0x8009B5A0u,4,timer-1);
 if(l(0x8009B574u,4)==2){
  uint32_t state=l(0x8009B578u,4);
  switch(state){
  case 12:
   if(l(0x8009B6A4u,4)){s(0x801FEFF0u,1,0);issue(14,0x801FEFF0u,32,l,s,c);s(0x8009B6A4u,4,0);}
   else{issue(1,0,32,l,s,c);s(0x8009B6A4u,4,1);}break;
  case 13:s(0x8009B6A4u,4,0);issue(1,0,32,l,s,c);break;
  case 14:{uint32_t phase=l(0x8009B57Cu,4);
   if(phase==22){s(0x8009B58Cu,4,l(0x8009B58Cu,4)+1u);}
   if(phase>=21 && phase<=24)issue(phase==23?19:1,0,32,l,s,c);break;}
  case 15:if(l(0x8009B594u,4)==0){s(0x8009B574u,4,1);s(0x8009B578u,4,11);}break;
  case 16:case 17:
   if(l(state==16?0x8009B588u:0x8009B58Au,1)){s(0x8009B574u,4,1);s(0x8009B578u,4,11);}
   else issue(1,0,32,l,s,c);break;
  }
 }
 if(l(0x800A36A0u,4) && l(0x8009B554u,4))c(l(0x800A36A0u,4),0,0);
 uint32_t lane=l(0x8009B574u,4);
 if(lane==1){if(!l(0x8009B58Bu,1))issue(1,0,33,l,s,c);}
 else if(lane==3)issue(1,0,33,l,s,c);
}
