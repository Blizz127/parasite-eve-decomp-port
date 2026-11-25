/* Independent retail caller draft from original-code traces; test-only. */
#include <stdint.h>
typedef uint32_t (*call_fn)(uint32_t,uint32_t);
typedef void (*store_fn)(uint32_t,uint32_t,uint32_t);
uint32_t draft(call_fn call,store_fn store){
 if(call(0x80080940,0))return 0;
 store(0x800B8AB0,4,0);store(0x800B8AB4,4,0);store(0x800B8AB8,4,0);
 store(0x800A3530,4,0);store(0x800A3520,4,0);store(0x800A3510,4,0);
 store(0x800A3534,1,0);store(0x800A3524,1,0);store(0x800A3514,1,0);
 for(uint32_t i=5;i<=12;i++){
  store(0x800A3510+i,1,0);store(0x800A3520+i,1,0);store(0x800A3530+i,1,0);
 }
 for(uint32_t i=0;i<8;i++)call(0x8007E594,0x800A3540+24*i);
 store(0x800A3604,4,0);store(0x800A3600,4,0);store(0x800A3608,4,0);
 for(uint32_t i=0;i<8;i++)store(0x800A3680-16*i,4,0);
 store(0x800A3690,4,0);
 call(0x8007F994,0);call(0x8007FBCC,0x8007E964);call(0x8007FBD8,0x8007F88C);
 call(0x8007FBE4,0x8007F960);call(0x8007FBC0,0x8007F7E8);
 call(0x800822BC,0);call(0x80081E5C,0);return 1;
}
