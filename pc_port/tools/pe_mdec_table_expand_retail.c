#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);typedef void(*Store)(uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t out,Load load,Store store){
 uint32_t source=0x8010cbfcu,cur=out,distance=0;
 for(;;){
  uint32_t token=load(source++,1);
  if(token<0xf0u){
   for(uint32_t n=0;n<=token;n++){
    uint32_t value=distance?load(cur-distance,1):load(source++,1);
    store(cur++,value,1);
   }
  }else if(token==0xf0u)distance=0;
  else {distance=((token<<8)|load(source++,1))+0xffff0f01u;if(distance==0xf00u)break;}
 }
 for(uint32_t i=0;i<0x87fcu;i++){
  uint32_t at=out+8u+i*2u;
  uint32_t current=load(at,2);
  uint32_t previous=load(at-8u,2);
  uint32_t value=current^previous;
  store(at,value,2);
 }
 return 1;
}
