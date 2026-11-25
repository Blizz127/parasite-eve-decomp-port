/* Test-only model: finite RAM-backed device fixtures and scripted callees.
 * Uses mutable globals and sentinels; not a production provider. */
#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);typedef void(*Store)(uint32_t,uint32_t,uint32_t);
typedef uint32_t(*Call3)(uint32_t,uint32_t,uint32_t);typedef uint32_t(*Call8)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
static Call8 upstream;static Load active_ld;static Store active_st;static uint32_t last_a1,last_a3,route_entry,space_entry;
static uint32_t part_c214(void){
 uint32_t idx=active_ld(0x800be9e4u,4),base=active_ld(0x800c0dc8u,4),entry=base+(idx<<5);active_st(entry,2,2);
 for(uint32_t i=0;i<4;i++){uint32_t v=active_ld(entry+28u+i,1);active_st(0x800a3490u+i,v,1);}
 uint32_t value=active_ld(entry+8u,4),next=active_ld(0x800be998u,4),cb=active_ld(0x800b0cc8u,4);active_st(0x800a3494u,value,4);active_st(0x800be9e4u,next,4);
 if(cb)value=upstream(cb,cb,last_a1,0x800a3490u,last_a3,0,0,0);
 active_st(0x800b89f4u,0,4);return value;
}
static uint32_t forward8(uint32_t a,uint32_t b,uint32_t c,uint32_t d,uint32_t e,uint32_t f,uint32_t g,uint32_t h){if(a==0x8007c214u)return part_c214();last_a1=c;last_a3=e;if(a==0x8007ce80u){for(uint32_t i=0;i<d;i++){uint32_t v=active_ld(c+4u*i,4);active_st(b+4u*i,v,4);}last_a1=c+4u*d;return 0;}
 if(a==0x8007ceacu){uint32_t chcr=0x1f8010b8u,v=active_ld(chcr,4);while(v&0x01000000u)v=active_ld(chcr,4);uint32_t p=active_ld(0x8009b348u,4);v=active_ld(p+2u,1);active_st(p+2u,(g&255u)==1u?v|8u:v&~8u,1);p=active_ld(0x8009b348u,4);(void)active_ld(p,4);p=active_ld(0x8009b344u,4);v=active_ld(p,4);active_st(p,v|0x8000u,4);active_st(0x1f8010b0u,c,4);active_st(0x1f8010b4u,(d<<16)|e,4);p=active_ld(0x8009b32cu,4);while(!(active_ld(p,1)&0x40u)){}active_st(chcr,f,4);v=active_ld(chcr,4);last_a1=chcr;last_a3=e;return v;}
 return upstream(a,b,c,d,e,f,g,h);}
static uint32_t forward3(uint32_t a,uint32_t b,uint32_t c){last_a1=c;if(a==0x8007c444u){for(uint32_t i=0;i<c;i++){uint32_t base=active_ld(0x800c0dc8u,4);active_st(base+((b+i)<<5),0,4);}return 0;}return upstream(a,b,c,0,0,0,0,0);}
static uint32_t part_prefix(Load ld,Store st){
 uint32_t x=ld(0x800b89f4u,4);if(x==1u)return 1;
 if(ld(0x800a801cu,4)){
  uint32_t p=ld(0x8009b34cu,4);if(ld(p,4)&0x1000000u){
   x=ld(0x800c0db8u,4);st(0x800b0cd0u,1u,4);
   if(x){x=ld(0x800bcd7cu,4)+1u;st(0x800bcd7cu,x,4);}st(0x8009b374u,1u,4);return x;
  }
 }
 return 0x100u;
}

static uint32_t part_response(uint32_t result,Load ld,Store st){
 if(result==5u)return 5u;
 uint32_t a=ld(0x801feff0u,1),b=ld(0x801feff1u,1);st(0x801fefe2u,a,2);st(0x801fefe4u,b,2);
 if(ld(0x801fefe2u,2)&4u){st(0x8009b374u,3u,4);return 3u;}return 0x100u;
}

static uint32_t part_ring(Load ld,Store st){
 uint32_t idx=ld(0x800be998u,4),base=ld(0x800c0dc8u,4),entry=base+(idx<<5);st(0x800a34a0u,entry,4);
 if(!ld(entry,2))return 0x100u;
 if(ld(0x800c0db8u,4)){uint32_t n=ld(0x800bcd7cu,4)+1u;st(0x800bcd7cu,n,4);}
 st(0x8009b374u,4u,4);return 4u;
}

static uint32_t part_device(Load ld,Store st,Call8 call){
 uint32_t p=ld(0x8009b32cu,4);st(p,0,1);p=ld(0x8009b338u,4);st(p,0,1);
 p=ld(0x8009b32cu,4);st(p,0,1);p=ld(0x8009b338u,4);st(p,0x80,1);
 p=ld(0x8009b33cu,4);st(p,0x20943,4);p=ld(0x8009b340u,4);st(p,0x1323,4);
 if(!ld(0x800a8020u,4)){
  for(uint32_t i=0;i<4;i++){p=ld(0x8009b334u,4);uint32_t v=ld(p,1);st(0x801fefe8u+i,v,1);}
  p=ld(0x8009b334u,4);for(uint32_t i=0;i<8;i++)(void)ld(p,1);
 }
 uint32_t pending=ld(0x800c0db8u,4);
 if(pending){uint32_t counter=ld(0x800bcd7cu,4),entry=ld(0x800a34a0u,4);return call(0x8007ce80u,entry,pending+(counter<<11),8,0,0,0,0);}
 uint32_t entry=ld(0x800a34a0u,4);st(0x801fefd0u,0x11000000u,4);st(0x801fefd4u,0,4);st(0x801fefd8u,0,4);return call(0x8007ceacu,3,entry,0,8,0x11000000u,0,0);
}

static uint32_t part_wait(Load ld,Store st){
 uint32_t p=ld(0x8009b35cu,4);while(ld(p,4)&0x1000000u){}
 uint32_t entry=ld(0x800a34a0u,4),control=ld(0x8009b33cu,4);
 last_a1=0;for(uint32_t i=0;i<4;i++){uint32_t v=ld(0x801fefe8u+i,1);last_a1|=v<<(8u*i);st(entry+0x1cu+i,v,1);}
 st(control,0x20843u,4);p=ld(0x8009b340u,4);st(p,0x1325u,4);return 0x100u;
}

static uint32_t part_filter(Load ld,Store st){
 if(ld(0x800c0dc0u,4)==1u){
  uint32_t want=ld(0x800b6918u,4);
  if(want){uint32_t entry=ld(0x800a34a0u,4);if(want!=ld(entry+8u,2)){
   st(entry,0,2);uint32_t pending=ld(0x800c0db8u,4);
   if(!pending)return 0;
   uint32_t counter=ld(0x800bcd7cu,4)+1u;st(0x800bcd7cu,counter,4);return counter;
  }st(0x800c0dc0u,0,4);}
 }
 return 0x100u;
}

static uint32_t part_route(Load ld,Store st){
 uint32_t entry=ld(0x800a34a0u,4);route_entry=entry;uint32_t flag=ld(entry,2);
 if(flag==0x160u){uint32_t header=ld(entry+2u,2),expected=ld(0x800b8620u,4);if(((header>>10)&31u)==expected)return 0x100u;}
 if(ld(0x800c0db8u,4))st(0x800bcd7cu,0,4);else (void)ld(entry,2);
 entry=ld(0x800a34a0u,4);st(0x8009b374u,5u,4);st(entry,0,2);return 5;
}

static uint32_t part_sequence(uint32_t entry,Load ld,Store st,Call3 call){
 int32_t old=(int16_t)ld(0x800a8018u,2);uint32_t seq=ld(entry+4u,2);
 if(old==(int32_t)seq){uint32_t target=ld(0x800a5d54u,4);if(!target)return 0x100u;if(target==ld(entry+8u,2))return 0x100u;}
 uint32_t start=ld(0x800be9e4u,4),end=ld(0x800be998u,4);st(0x800a5d54u,0,4);st(0x800a8018u,0,2);(void)call(0x8007c444u,start,end-start);
 start=ld(0x800be9e4u,4);entry=ld(0x800a34a0u,4);st(0x800be998u,start,4);st(entry,0,2);
 if(ld(0x800c0db8u,4)){uint32_t n=ld(0x800bcd7cu,4)+1u;st(0x800bcd7cu,n,4);}st(0x8009b374u,6u,4);return 6;
}

static uint32_t part_limit(Load ld,Store st){
 uint32_t entry=ld(0x800a34a0u,4);if(ld(entry+4u,2))return 0x101u;
 uint32_t value=ld(entry+8u,2),limit=ld(0x800c0dbcu,4);st(0x800a8018u,0,2);st(0x800a5d54u,value,4);
 return !limit||value<limit?0x102u:0x103u;
}

static uint32_t part_overflow(Load ld,Store st,Call3 call){
 uint32_t start=ld(0x800be9e4u,4),end=ld(0x800be998u,4),delta=end-start;
 st(0x800a5d54u,0,4);st(0x800a8018u,0,2);(void)call(0x8007c444u,start,delta);
 uint32_t next=ld(0x800be9e4u,4),entry=ld(0x800a34a0u,4);st(0x800be998u,next,4);st(entry,0,2);
 uint32_t cb=ld(0x800b0cccu,4);st(0x800c0dc0u,1u,4);if(cb)(void)call(cb,start,delta);
 if(ld(0x800c0db8u,4)){uint32_t n=ld(0x800bcd7cu,4)+1u;st(0x800bcd7cu,n,4);}st(0x8009b374u,7u,4);return 7;
}

static uint32_t part_space(uint32_t a1,Load ld,Store st,Call3 call){
 uint32_t size=ld(0x800c20c4u,4),idx=ld(0x800be998u,4),entry=ld(0x800a34a0u,4);space_entry=entry;uint32_t count=ld(entry+6u,2);
 if((uint32_t)(size-idx-1u)>=count)return 0x101u;
 if(ld(0x800c0dbcu,4))return 0x102u;
 st(entry,1,2);uint32_t cb=ld(0x800b0cccu,4);st(0x800c0dc0u,1,4);if(cb)(void)call(cb,entry,a1);
 if(ld(0x800c0db8u,4)){uint32_t n=ld(0x800bcd7cu,4)+1u;st(0x800bcd7cu,n,4);}st(0x8009b374u,8u,4);return 8;
}

static uint32_t part_wrap(uint32_t entry,Load ld,Store st){
 uint32_t base=ld(0x800c0dc8u,4);if(ld(base,2)){
  st(entry,0,2);if(ld(0x800c0db8u,4)){uint32_t n=ld(0x800bcd7cu,4)+1u;st(0x800bcd7cu,n,4);}st(0x8009b374u,9u,4);return 9;
 }
 st(entry,1,2);base=ld(0x800c0dc8u,4);uint32_t src=ld(0x800a34a0u,4);st(0x800be998u,0,4);
 for(uint32_t i=0;i<8;i++){uint32_t v=ld(src+4u*i,4);st(base+4u*i,v,4);}
 base=ld(0x800c0dc8u,4);st(0x800a34a0u,base,4);return 0x100u;
}

static uint32_t part_advance(uint32_t wrap,Load ld,Store st){
 if(wrap){uint32_t n=ld(0x800be998u,4);st(0x800be9e4u,n,4);}st(0x8009b374u,10u,4);
 uint32_t seq=ld(0x800a8018u,2),size=ld(0x800c20c4u,4),base=ld(0x800c0dc8u,4),idx=ld(0x800be998u,4);st(0x800a8018u,seq+1u,2);
 uint32_t mode=ld(0x800a801cu,4);st(0x800c0dc4u,base+(size<<5)+(idx*2016u),4);
 uint32_t p=ld(0x8009b33cu,4);
 if(mode){st(p,0x20943u,4);p=ld(0x8009b340u,4);st(p,0x1323u,4);return 0x11000000u;}
 st(p,0x21020843u,4);return 0x11400100u;
}

static uint32_t part_final(uint32_t t0,Load ld,Store st,Call8 call){
 uint32_t e=ld(0x800a34a0u,4),n=ld(e+6u,2),seq=ld(e+4u,2);int last=(n-1u)==seq;uint32_t pending=ld(0x800c0db8u,4);
 if(last)st(0x800b89f4u,1,4);
 if(pending){uint32_t count=ld(0x800bcd7cu,4),dest=ld(0x800c0dc4u,4);(void)call(0x8007ce80u,dest,pending+(count<<11)+0x20u,0x1f8u,last?1u:0u,0,0,0);count=ld(0x800bcd7cu,4)+1u;st(0x800bcd7cu,count,4);}
 else {uint32_t dest=ld(0x800c0dc4u,4);st(0x801fefd0u,t0,4);st(0x801fefd4u,last?1u:0u,4);st(0x801fefd8u,0,4);(void)call(0x8007ceacu,3,dest,0,0x1f8u,t0,last?1u:0u,0);}
 if(last){uint32_t v=ld(0x800b6914u,4);st(0x800a8018u,0,2);st(0x800a5d54u,0,4);st(0x800b8620u,v,4);}
 uint32_t p=ld(0x8009b340u,4);st(p,0x1325u,4);e=ld(0x800a34a0u,4);st(e,3u,2);uint32_t idx=ld(0x800be998u,4),pending2=ld(0x800c0db8u,4);idx++;st(0x800be998u,idx,4);
 if(!pending2)return idx;
 uint32_t flag=ld(0x800b89f4u,4);
 if(!flag)return 0;
 return call(0x8007c214u,0,0,0,0,0,0,0);
}

uint32_t draft(Load ld,Store st,Call8 call){
 upstream=call;active_ld=ld;active_st=st;last_a1=last_a3=route_entry=space_entry=0;uint32_t x=part_prefix(ld,st);if(x!=0x100u)return x;
 x=forward8(0x8007b290u,1u,0x801feff0u,0,0,0,0,0);x=part_response(x,ld,st);if(x!=0x100u)return x;
 x=part_ring(ld,st);if(x!=0x100u)return x;
 (void)part_device(ld,st,forward8);(void)part_wait(ld,st);
 x=part_filter(ld,st);if(x!=0x100u)return x;
 x=part_route(ld,st);if(x!=0x100u)return x;
 x=part_sequence(route_entry,ld,st,forward3);if(x!=0x100u)return x;
 x=part_limit(ld,st);uint32_t fromwrap=1;
 if(x==0x103u)return part_overflow(ld,st,forward3);
 if(x==0x101u)fromwrap=0;
 else {x=part_space(last_a1,ld,st,forward3);if(x==8u)return x;if(x==0x102u){x=part_wrap(space_entry,ld,st);if(x!=0x100u)return x;}}
 uint32_t t0=part_advance(fromwrap,ld,st);return part_final(t0,ld,st,forward8);
}
