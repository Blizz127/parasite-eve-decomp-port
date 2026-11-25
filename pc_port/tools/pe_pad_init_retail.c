#include <stdint.h>
typedef uint32_t (*Call)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t a,uint32_t b,uint32_t c,uint32_t d,Call call)
{return call(0x80082CF0u,a,b,c,d);}
