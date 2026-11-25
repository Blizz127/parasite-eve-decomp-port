#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);
typedef void(*Store)(uint32_t,uint32_t,uint32_t);
typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t a,uint32_t b,Load L,Store S,Call C)
{
 S(0x8009B75C,4,0);S(0x8009B770,4,0);C(0x80084B44,0,0);
 S(0x8009B724,4,0x800846AC);S(0x8009B728,4,0x80084644);
 S(0x8009B72C,4,0x800847B0);S(0x8009B730,4,0x8008486C);
 S(0x8009B734,4,0x80084AE8);S(0x8009B738,4,0x80084B20);
 S(0x8009B758,4,0x800A5B70);S(0x8009B748,4,0x800847A0);
 C(0x80071A24,0x800A5B70,480);
 S(0x800A5BA0,4,a);S(0x800A5C90,4,b);
 for(uint32_t i=0;i<2;i++) {
  uint32_t slot=0x800A5B70+i*240;
  S(slot+12,4,0);S(slot+16,4,slot);
  S(L(slot+48,4),1,255);S(L(slot+48,4)+1,1,0);
  S(slot+60,4,0x800A5AE0+i*35);S(slot+64,4,0x800A5B28+i*35);
  for(uint32_t j=0;j<6;j++)S(slot+93+j,1,255);
 }
 C(0x80082ADC,0,0);S(0x8009B75C,4,1);return 1;
}
