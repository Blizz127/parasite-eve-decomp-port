#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);
typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t rect,uint32_t r,uint32_t g,uint32_t b,Load L,Call C) {
 C(0x80074E28,0x800118BC,rect,0,0);
 uint32_t color=(r&255)|((g&255)<<8)|((b&255)<<16);
 uint32_t table=L(0x80095744,4),operation=L(table+12,4),target=L(table+8,4);
 return C(target,operation,rect,8,color);
}
