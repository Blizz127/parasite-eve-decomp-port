#include <stdint.h>
typedef uint32_t (*Load)(uint32_t,uint32_t);
typedef void (*Store)(uint32_t,uint32_t);
typedef uint32_t (*Call)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t ot,uint32_t count,Load load,Store store,Call call)
{
    if(load(0x8009574Eu,1)>=2u)
        call(load(0x80095748u,4),0x80011910u,ot,count,0);
    uint32_t table=load(0x80095744u,4);
    call(load(table+44u,4),ot,count,0,0);
    store(0x8009580Cu,0x040957F8u);
    store(ot,0x0009580Cu);
    return ot;
}
