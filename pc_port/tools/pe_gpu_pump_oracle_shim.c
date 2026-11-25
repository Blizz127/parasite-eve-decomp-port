/* Synthetic devices for independent pump vs original-execution oracle. */
#include <stdint.h>
#include <string.h>
#include "psx_compat.h"
static uint32_t ram[0x200000/4], chcr, status, mask, issue, nworker, ndraw, ndereg;
static uint32_t calls[64][3]; static PeGpuPumpTrace tr; static uint64_t order;
uint32_t PE_LoadU32(pe_addr_t a){return ram[(a&0x1fffffu)/4u];}
void PE_StoreU32(pe_addr_t a,uint32_t v){ram[(a&0x1fffffu)/4u]=v;}
uint32_t PE_GPU_ReadDMA2CHCR(void){return chcr;}
uint32_t PE_GPU_ReadStatus(void){return status;}
uint16_t pe_plat_irq_set_mask(uint16_t v){uint16_t old=mask;mask=v;return old;}
uint32_t pe_plat_dma_callback(uint32_t c,uint32_t h){(void)c;(void)h;++ndereg;return 0;}
PeGpuPumpTrace *pe_plat_pump_trace_existing(void){return &tr;}
void pe_plat_pump_enter_existing(void){}
uint64_t pe_plat_pump_next_existing(void){return ++order;}
int pe_plat_pump_load_existing(uint32_t c,uint32_t field,uint32_t *v){*v=PE_LoadU32(0x800bd030u+c*0x60u+field);return 1;}
int pe_plat_pump_worker_existing(pe_addr_t w,pe_addr_t a,uint32_t x,uint32_t c){(void)c;calls[nworker][0]=w;calls[nworker][1]=a;calls[nworker++][2]=x;if(issue)chcr|=0x1000000u;return 1;}
int pe_plat_pump_drawsync_existing(pe_addr_t c){(void)c;++ndraw;return 1;}
extern int PE_func_80076EE4_Pump(int *);
void run_case(uint32_t consumer,uint32_t count,uint32_t busy,uint32_t dma_issue,uint32_t marker,uint32_t callback,uint32_t imask,uint32_t seed,uint32_t *out){
 memset(ram,0,sizeof(ram));memset(&tr,0,sizeof(tr));memset(calls,0,sizeof(calls));order=0;nworker=ndraw=ndereg=0;mask=imask;issue=dma_issue;status=0x04000000u;chcr=0x201u|(busy?0x1000000u:0);
 PE_StoreU32(0x80095878u,consumer);PE_StoreU32(0x80095874u,(consumer+count)&63u);PE_StoreU32(0x80095754u,marker);PE_StoreU32(0x80095758u,callback);
 for(uint32_t j=0;j<count;j++){uint32_t slot=(consumer+j)&63u,base=0x800bd030u+slot*0x60u;PE_StoreU32(base,0x80076664u);PE_StoreU32(base+4u,base+12u);PE_StoreU32(base+8u,0x80010000u+((seed+j*0x100u)&0xfffcu));}
 int returned=0,result=PE_func_80076EE4_Pump(&returned);
 out[0]=result;out[1]=returned;out[2]=PE_LoadU32(0x80095878u);out[3]=mask;out[4]=PE_LoadU32(0x80095754u);out[5]=nworker;out[6]=ndereg;out[7]=ndraw;out[8]=!!(chcr&0x1000000u);out[9]=PE_LoadU32(0x80095880u);
 for(uint32_t j=0;j<nworker;j++){out[10+j*3]=calls[j][0];out[11+j*3]=calls[j][1];out[12+j*3]=calls[j][2];}
}
