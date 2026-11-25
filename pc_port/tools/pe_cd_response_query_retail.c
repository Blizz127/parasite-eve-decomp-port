/* Test-only caller model inferred from original-code traces. */
#include <stdint.h>
typedef uint32_t (*Load)(uint32_t,uint32_t);
typedef void (*Store)(uint32_t,uint32_t,uint32_t);
typedef uint32_t (*Call)(uint32_t,uint32_t,uint32_t);
uint32_t draft(uint32_t sequence,uint32_t response,Load load,Store store,Call call)
{
    uint32_t index,record;
    if(sequence){
        call(0x8007FC64u,0,0);
        index=load(0x800A3690u,4);
        unsigned n;
        for(n=0;n<8;n++){
            if(load(0x800A3610u+16u*index,4)==sequence)break;
            index=(index+1u)&7u;
        }
        if(n==8){
            index=load(0x800A3690u,4);
            if((int32_t)load(0x800A3610u+16u*index,4)<(int32_t)sequence)return 0;
        }
        index=(load(0x800A3690u,4)-1u)&7u;
        for(n=0;n<8;n++){
            if(load(0x800A3610u+16u*index,4)==sequence)break;
            index=(index-1u)&7u;
        }
        if(n==8)return 6;
    }else{
        index=(load(0x800A3690u,4)-1u)&7u;
        if(!load(0x800A3610u+16u*index,4))return 6;
    }
    record=0x800A3610u+16u*index;
    for(unsigned n=0;n<4;n++)store(0x800A3500u+4u*n,4,load(record+4u*n,4));
    call(0x80080998u,response,0x800A3505u);
    return load(0x800A3504u,1);
}
