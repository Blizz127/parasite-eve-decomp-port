#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_sdk.h"
#include "pe_plat/spu_dma_hw.h"
#define GA_OVERLAY 0x800B0CD8u
#define GA_XA_TABLE 0x8009317Cu
#define GA_PEIMG_LBA 0x800B0DD8u
#define GA_GP_400 0x8009D170u
#define GA_GP_404 0x8009D174u
#define GA_GP_408 0x8009D178u
#define GA_GP_40C 0x8009D17Cu
extern int func_80087414(void);
extern int func_8006E6D4(int,int,pe_addr_t,int);
extern int func_8006E7E8(void);

/* DAY1-1: original complete181-word loop, not one dispatch per host call.
 * stack_flag bit0 selects blocking retry; successful intermediate states
 * continue internally even for nonblocking calls. The provider functions
 * retain their host disc/SPU hardware adaptations. */
int func_8006CDA4(int a0, int a1, int a2, pe_addr_t a3, int stack_len,
                  int stack_flag)
{
    int result=-1;
    for (;;) {
        /* Successful state transitions use continue, bypassing the bottom
         * check. A native callback stop must precede the next state's I/O. */
        if (PE_Port_ShouldStop()) return 1;
        unsigned f0=PE_LoadU8(GA_OVERLAY+0xF0u);
        switch (f0) {
        case 0: {
            uint32_t index=(uint32_t)a1<<1u;
            uint32_t start=PE_LoadU16(GA_XA_TABLE+4u+index);
            uint32_t count=PE_LoadU16(GA_XA_TABLE+6u+index)-start;
            PE_StoreU32(GA_GP_404,count);
            PE_StoreU32(GA_GP_408,count);
            PE_StoreU32(GA_GP_400,PE_LoadU32(GA_PEIMG_LBA)+PE_LoadU32(GA_XA_TABLE)+start);
            if (a0==0) result=func_80087198();
            else if (a0==3) result=func_80087414();
            if (PE_Port_ShouldStop()) return 1;
            PE_StoreU8(GA_OVERLAY+0xF0u,7u);
            if ((a0!=0 && a0!=3) || result!=-1) continue;
            break;
        }
        case 7: {
            uint32_t remain=PE_LoadU32(GA_GP_408),chunk=remain;
            if (!remain) {
                PE_StoreU8(GA_OVERLAY+0xF0u,0u);
                return 0;
            }
            if ((uint32_t)stack_len<chunk) chunk=(uint32_t)stack_len;
            PE_StoreU32(GA_GP_40C,chunk);
            result=func_8006E6D4((int)PE_LoadU32(GA_GP_400),
                    (int)(PE_LoadU32(GA_GP_404)-remain),a3,(int)chunk);
            if (PE_Port_ShouldStop()) return 1;
            if (result!=-1) PE_StoreU8(GA_OVERLAY+0xF0u,8u);
            /* A failed read retains state7 for retry; it is not completion. */
            break;
        }
        case 8:
            result=func_8006E7E8();
            if (PE_Port_ShouldStop()) return 1;
            if (result==-1) PE_StoreU8(GA_OVERLAY+0xF0u,7u);
            else if (!result) {
                PE_StoreU8(GA_OVERLAY+0xF0u,9u);
                continue;
            }
            break;
        case 9:
            if (a0==0) result=func_800871AC(a3,PE_LoadU32(GA_GP_40C)<<11u);
            else if (a0==1) result=func_80087090(a3,0);
            else if (a0==2) result=func_800875FC((unsigned)a2,a3);
            else if (a0==3) result=func_80087428((unsigned)a2,a3,PE_LoadU32(GA_GP_40C)<<11u);
            if (PE_Port_ShouldStop()) return 1;
            if (result==-1) PE_StoreU8(GA_OVERLAY+0xF0u,0u);
            else {
                PE_StoreU8(GA_OVERLAY+0xF0u,10u);
                continue;
            }
            break;
        case 10:
            result=func_800870E0();
            if (PE_Port_ShouldStop()) return 1;
            if (result==-1) PE_StoreU8(GA_OVERLAY+0xF0u,0u);
            else if (!result) {
                PE_StoreU32(GA_GP_408,PE_LoadU32(GA_GP_408)-PE_LoadU32(GA_GP_40C));
                PE_StoreU8(GA_OVERLAY+0xF0u,7u);
            }
            break;
        default:
            /* Original unused states spin without progress. Keep corrupt/
             * unsupported state explicit instead of reporting completion. */
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 1;
        }
        if (!(stack_flag&1)) return 1;
        if (PE_Port_ShouldStop()) return 1;
        /* Retail blocking polls 870E0 while DMA4 IRQ clears D_8009D24C.
         * Host completion is PE_SpuDma_Service, normally pumped at VSync;
         * this loop never reaches VSync, so the IRQ must run here.  A stop
         * from its callback dispatch ends the loop before the next state. */
        (void)PE_SpuDma_Service();
        if (PE_Port_ShouldStop()) return 1;
    }
}
