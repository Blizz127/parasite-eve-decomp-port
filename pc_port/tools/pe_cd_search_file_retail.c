/* Independent scratch retail caller draft, synthetic original-code observations.
 * Not production; guest path convention differs from the host adapter. */
#include <stdint.h>
typedef uint32_t (*call_fn)(uint32_t,uint32_t,uint32_t);
typedef void (*store_fn)(uint32_t,uint32_t,uint32_t);
static uint8_t *m; static call_fn call; static store_fn trace;
static uint8_t r8(uint32_t a){return m[a&0x1fffff];}
static uint32_t r32(uint32_t a){return r8(a)|(uint32_t)r8(a+1)<<8|(uint32_t)r8(a+2)<<16|(uint32_t)r8(a+3)<<24;}
static void w8(uint32_t a,uint8_t v){m[a&0x1fffff]=v;}
static void w32(uint32_t a,uint32_t v){for(int i=0;i<4;i++)w8(a+i,v>>(8*i));trace(a,4,v);}
uint32_t draft(uint8_t *ram,call_fn cb,store_fn st,uint32_t out,uint32_t path){
 m=ram;call=cb;trace=st;
 const uint32_t buf=0x801FEB80u;uint32_t p=path,parent=1,n=0;
 if((int32_t)r32(0x8009B6E0)<(int32_t)call(0x8007F7A8,0,0)){
  if(!call(0x80081714,0,0))return 0;
  w32(0x8009B6E0,call(0x8007F7A8,0,0));
 }
 if(r8(p)!=0x5c)return 0;
 while(r8(p)){
  n=0;
  while(r8(p) && r8(p)!=0x5c){w8(buf+n++,r8(p++));}
  w8(buf+n,0);
  if(r8(p)!=0x5c)break;
  parent=call(0x800819D8,parent,buf);
  if(parent==0xffffffffu)break;
  p++;w8(buf,0);
 }
 if(parent==0xffffffffu || !n){
  if((int32_t)r32(0x8009AFC0)>0)call(0x80071A74,0x80011E88,path);
  return 0;
 }
 if(!call(0x80081A7C,parent,0x5c)){
  if((int32_t)r32(0x8009AFC0)>0)call(0x80073C5C,0x80011EA0,0);
  return 0;
 }
 if((int32_t)r32(0x8009AFC0)>1)call(0x80071A74,0x80011EBC,buf);
 for(uint32_t a=0x800A36B0;r8(a+8);a+=24){
  if(call(0x800816F4,a+8,buf)){
   if((int32_t)r32(0x8009AFC0)>1)call(0x80071A74,0x80011EDC,buf);
   uint32_t a0=r32(a),a1=r32(a+4),a2=r32(a+8),a3=r32(a+12);
   w32(out,a0);w32(out+4,a1);w32(out+8,a2);w32(out+12,a3);
   a0=r32(a+16);a1=r32(a+20);w32(out+16,a0);w32(out+20,a1);
   return a;
  }
 }
 if((int32_t)r32(0x8009AFC0)>0)call(0x80071A74,0x80011EE8,buf);
 return 0;
}
