#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);
typedef void(*Store)(uint32_t,uint32_t);
typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t rect,uint32_t x,uint32_t y,Load load,Store store,Call call)
{
    call(0x80074E28u,0x800118ECu,rect,0,0);
    if(load(rect+4u,2u)==0u)return 0xFFFFFFFFu;
    if(load(rect+6u,2u)==0u)return 0xFFFFFFFFu;
    uint32_t position=(y<<16)|(x&0xFFFFu);
    uint32_t source=load(rect,4u),table=load(0x80095744u,4u);
    store(0x800957F0u,position);
    store(0x800957ECu,source);
    uint32_t size=load(rect+4u,4u);
    store(0x800957F4u,size);
    uint32_t worker=load(table+24u,4u),target=load(table+8u,4u);
    return call(target,worker,0x800957E4u,20u,0u);
}
