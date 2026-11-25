#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);
typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t packet,Load L,Call C) {
 uint32_t table=L(0x80095744,4),sync=L(table+60,4),words=L(packet+3,1);
 C(sync,0,0);
 table=L(0x80095744,4);
 return C(L(table+20,4),packet+4,words);
}
