#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);
typedef void(*Store)(uint32_t,uint32_t);
typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t env,Load L,Store S,Call C) {
 if(L(0x8009574E,1)>=2)C(L(0x80095748,4),0x8001193C,env,0,0);
 uint32_t packet=env+28;
 C(0x80075EE0,packet,env,0,0);
 uint32_t tag=L(packet,4),table=L(0x80095744,4);
 S(packet,tag|0x00FFFFFF);
 uint32_t worker=L(table+24,4),target=L(table+8,4);
 C(target,worker,packet,64,0);
 C(0x80071A34,0x8009575C,env,92,0);
 return env;
}
