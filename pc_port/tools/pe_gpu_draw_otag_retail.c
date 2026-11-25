#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);
typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t packet,Load L,Call C) {
 if(L(0x8009574E,1)>=2)C(L(0x80095748,4),0x80011928,packet,0,0);
 uint32_t table=L(0x80095744,4),operation=L(table+24,4),target=L(table+8,4);
 return C(target,operation,packet,0,0);
}
