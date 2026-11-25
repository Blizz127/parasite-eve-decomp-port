#include <stdint.h>
typedef uint32_t(*Load)(uint32_t,uint32_t);
typedef void(*Store)(uint32_t,uint32_t,uint32_t);
typedef uint32_t(*Call)(uint32_t,uint32_t,uint32_t,uint32_t,uint32_t);
uint32_t draft(Load load,Store store,Call call)
{
    call(0x80072714u,0,0,0,0);
    uint32_t state=load(0x8009AFD8u,4);
    if(state==1u){call(0x800824F0u,0,0,0,0);call(0x800824C8u,0,0,0,0);}
    else{call(0x8007A8ECu,0,0,0,0);call(0x8007A4BCu,0,0,0,0);}
    uint32_t first=load(0x8009AF1Cu,4);store(first,0,1);
    uint32_t second=load(0x8009AF28u,4);store(second,0,1);
    return call(0x80072724u,0,0,0,0);
}
