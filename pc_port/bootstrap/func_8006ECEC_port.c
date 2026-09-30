/* Complete original 8006ECEC..8006F044 (214 words). Native translation,
 * not a matching-C claim. Loads the M0000I transition's two TIM banks and
 * executable overlay. Original/native provider-contract cases are separate
 * from acceptance of the loaded overlay's frame loop. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

/* Dead stack-local DsDataSync output, as in the adjacent 6E834 adapter. */
#define SYNC_LOCAL 0x801FFED0u

static int read_range(pe_addr_t table,pe_addr_t destination_slot,
                      uint32_t image_base,int reload_base)
{
retry:
    for (;;) {
        uint32_t first=PE_LoadU16(table),last=PE_LoadU16(table+2);
        uint32_t base=reload_base?PE_LoadU32(0x800B0DD8u):image_base;
        int result=func_8006E6D4((int32_t)(base+first),0,
                               PE_LoadU32(destination_slot),(int)(last-first));
        if(PE_Port_ShouldStop()) return 0;
        if(result!=-1) break;
    }
    for (;;) {
        int result=func_800811E4(SYNC_LOCAL);
        if(PE_Port_ShouldStop()) return 0;
        if((uint32_t)result+1u<2u) D_800B0CD8&=0xFEFFBFFFu;
        if(result==0) return 1;
        if(result==-1) goto retry;
    }
}

int func_8006ECEC(void)
{
    uint32_t image_base=PE_LoadU32(0x800B0DD8u);
    func_80073A44(0);
    if(PE_Port_ShouldStop()) return 0;
    func_80074D28(0);
    if(PE_Port_ShouldStop()) return 0;
    func_8006CDA4(1,204,0,PE_LoadU32(0x800B0E6Cu),33,1);
    if(PE_Port_ShouldStop()) return 0;
    if(!read_range(0x80093168u,0x800B0E34u,image_base,0)) return 0;
    pe_addr_t bank=PE_LoadU32(0x800B0E34u);
    for(unsigned i=0;i<3;i++) {
        func_800718D0(bank+PE_LoadU32(bank+i*4u));
        if(PE_Port_ShouldStop()) return 0;
    }
    pe_addr_t table=(PE_LoadU32(0x800A77FCu)&0x2000u)?0x8009316Au:0x8009316Cu;
    if(!read_range(table,0x800B0E44u,image_base,0)) return 0;
    bank=PE_LoadU32(0x800B0E44u);
    for(unsigned i=0;i<262;i++) {
        func_800718D0(bank+PE_LoadU32(bank+i*4u));
        if(PE_Port_ShouldStop()) return 0;
    }
    func_80074DC0(0);
    if(PE_Port_ShouldStop()) return 0;
    if(!read_range(0x8009316Eu,0x80011614u,image_base,1)) return 0;
    func_80072714();
    if(PE_Port_ShouldStop()) return 0;
    func_800726C4();
    if(PE_Port_ShouldStop()) return 0;
    func_80072724();
    return 0;
}
