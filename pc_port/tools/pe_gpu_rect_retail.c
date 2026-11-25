#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);
typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t label,uint32_t rect,Load L,Call C)
{
 uint32_t debug=L(0x8009574E,1),fmt;
 if(debug==1) {
  int32_t w=(int16_t)L(rect+4,2),mw=(int16_t)L(0x80095750,2),x,y,h,mh;
  if(w>mw)goto invalid;
  x=(int16_t)L(rect,2);if(x+w>mw)goto invalid;
  y=(int16_t)L(rect+2,2);mh=(int16_t)L(0x80095752,2);if(y>mh)goto invalid;
  h=(int16_t)L(rect+6,2);if(y+h>mh||x<0||y<0||w<=0||h<=0)goto invalid;
  return 0;
invalid:fmt=0x80011898;
 }else if(debug==2)fmt=0x800118B8;else return 2;
 C(L(0x80095748,4),fmt,label,0,0,0);
 uint32_t x=(uint32_t)(int32_t)(int16_t)L(rect,2);
 uint32_t y=(uint32_t)(int32_t)(int16_t)L(rect+2,2);
 uint32_t w=(uint32_t)(int32_t)(int16_t)L(rect+4,2);
 uint32_t h=(uint32_t)(int32_t)(int16_t)L(rect+6,2);
 return C(L(0x80095748,4),0x800118A4,x,y,w,h);
}
