#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
typedef uint32_t(*Load)(uint32_t,uint32_t);typedef uint32_t(*Hook)(uint32_t,uint32_t,uint32_t);
static Load load;static Hook hook;
void set_callbacks(Load l,Hook h){load=l;hook=h;}
uint32_t PE_LoadU32(pe_addr_t a){return load(a,4);}
uint8_t PE_LoadU8(pe_addr_t a){return load(a,1);}
bool PE_RangeIsRam(pe_addr_t a,size_t n){uint32_t p=a&0x1fffffffu;return p<0x200000u&&n<=0x200000u-p;}
int PE_MMIO_IsModelled(pe_addr_t a,size_t n){(void)a;(void)n;return 0;}
unsigned PE_Port_StopEpoch(void){return 0;}
int pe_plat_gpu_2700(int x){return (int)hook(0x80077144u,(uint32_t)x,0);}
int PE_GuestHook_Call(const char *s,const char*c,pe_addr_t fn,unsigned arity,uintptr_t a0,uintptr_t a1,uintptr_t a2,uintptr_t a3,int*ret){(void)s;(void)c;(void)arity;(void)a2;(void)a3;*ret=(int)hook(fn,(uint32_t)a0,(uint32_t)a1);return 1;}
void PE_GuestHook_Refuse(const char*s,const char*c,pe_addr_t f,unsigned arity,uintptr_t a0,uintptr_t a1,uintptr_t a2,uintptr_t a3,const char*w){(void)s;(void)c;(void)f;(void)arity;(void)a0;(void)a1;(void)a2;(void)a3;(void)w;}

int32_t PE_Bios_Printf(pe_addr_t fmt,const uint32_t *args,int n){(void)fmt;(void)args;(void)n;return 0;}
