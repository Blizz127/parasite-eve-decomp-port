/* Test-only original-code model; device reads are scripted. */
#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);typedef void(*Store)(uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t dst,uint32_t count,Load ld,Store st){
 uint32_t p=ld(0x8009b27cu,4);st(p,0,1);p=ld(0x8009b288u,4);st(p,0x80,1);
 p=ld(0x8009b2b0u,4);st(p,0x20943,4);p=ld(0x8009b28cu,4);st(p,0x1323,4);
 p=ld(0x8009b2b4u,4);uint32_t x=ld(p,4);st(p,x|0x8000,4);
 p=ld(0x8009b2b8u,4);st(p,dst,4);p=ld(0x8009b2bcu,4);st(p,count|0x10000,4);
 p=ld(0x8009b27cu,4);while(!(ld(p,1)&0x40)){}
 p=ld(0x8009b2c0u,4);st(p,0x11000000,4);p=ld(0x8009b2c0u,4);
 while(ld(p,4)&0x1000000){}p=ld(0x8009b28cu,4);st(p,0x1325,4);return 0;
}
