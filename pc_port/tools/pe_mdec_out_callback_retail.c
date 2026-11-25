#include <stdint.h>
typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t callback,Call call){return call(0x80073cf4u,1u,callback);}
