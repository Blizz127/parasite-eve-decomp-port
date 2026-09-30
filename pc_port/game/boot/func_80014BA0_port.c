/* Script76: original 80014BA0..80014DA0, all128 words. Turns the current
 * actor toward a 12-bit heading; task state retains direction/target while
 * the VM retries the instruction. Signed halfword and wrap tests follow
 * the original, including zero/negative steps and the exact4096 boundary. */
#include "pe_port_compat.h"

int func_80014BA0(pe_addr_t args)
{
    pe_addr_t actor=PE_LoadU32(0x8009D2F0u),task=PE_LoadU32(0x8009D300u);
    int current=PE_LoadU16(actor+0x3Au)&0xFFF;
    uint16_t flags=PE_LoadU16(task+8u);
    if(!(flags&0x20u)) {
        int target=(int)(PE_LoadU32(PE_LoadU32(args))&0xFFFu);
        int step=(int16_t)PE_LoadU16(PE_LoadU32(args+4u));
        if(current==target) return 1;
        PE_StoreU16(task+8u,(uint16_t)(flags|0x20u));
        int difference=target-current;
        if(difference<0)difference=-difference;
        if((target<current && difference<=2048) || (target>=current && difference>2048))step=-step;
        PE_StoreU32(task+0x14u,(uint32_t)step);
        PE_StoreU32(task+0x18u,(uint32_t)target);
    }
    uint32_t delta=PE_LoadU32(task+0x14u),target_word=PE_LoadU32(task+0x18u);
    int target=(int16_t)target_word,step=(int16_t)delta,next=current+step;
    if(current>=target && target>=next)goto finish;
    if(target>=current && next>=target)goto finish;
    if(step>0 && next>4096 && (next&0xFFF)>=target)goto finish;
    if(step<0 && next<0 && target>=(next&0xFFF))goto finish;
    PE_StoreU32(0x8009CE00u,PE_LoadU32(0x8009CE00u)-16u);
    PE_StoreU16(actor+0x3Au,(uint16_t)(PE_LoadU16(actor+0x3Au)+delta));
    PE_StoreU32(task+0x10u,1);
    return 0;
finish:
    PE_StoreU16(actor+0x3Au,(uint16_t)target_word);
    PE_StoreU16(task+8u,(uint16_t)(PE_LoadU16(task+8u)&0xFFDFu));
    return 1;
}
