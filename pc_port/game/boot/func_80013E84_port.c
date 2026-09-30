/* Script36: original13E84..14228 tracks and approaches another actor.
 * Task state latches target identity and turn rate across VM retries. */
#include "pe_port_compat.h"

int func_80013E84(pe_addr_t args)
{
    pe_addr_t task=PE_LoadU32(0x8009D300u),target,actor=PE_LoadU32(0x8009D2F0u);
    uint16_t flags=PE_LoadU16(task+8u);
    int32_t rate;
    if(!(flags&0x20u)) {
        uint32_t type=PE_LoadU32(PE_LoadU32(args));
        target=PE_LoadU32(type?0x8009D20Cu:0x8009D254u);
        if(type) {
            uint32_t id=PE_LoadU32(PE_LoadU32(args+4u));
            while(target && (PE_LoadU8(target+12u)!=type || PE_LoadU8(target+13u)!=id ||
                             (PE_LoadU32(target+0x98u)&0x10u)))target=PE_LoadU32(target+4u);
        }
        if(!target)return 1;
        PE_StoreU16(task+8u,(uint16_t)(flags|0x20u));
        rate=(int32_t)PE_LoadU32(PE_LoadU32(args+8u));
        PE_StoreU32(task+0x14u,(uint32_t)rate);PE_StoreU32(task+0x18u,target);
        PE_StoreU32(task+0x1Cu,PE_LoadU16(target+0x24u));
    } else {
        target=PE_LoadU32(task+0x18u);rate=(int32_t)PE_LoadU32(task+0x14u);
        if((PE_LoadU32(target+0x98u)&0x10u) || PE_LoadU16(target+0x24u)!=PE_LoadU32(task+0x1Cu)) {
            PE_StoreU16(task+8u,(uint16_t)(flags&0xFFDFu));return 1;
        }
    }
    uint32_t x=PE_LoadU32(actor+0x28u),z=PE_LoadU32(actor+0x30u);
    uint32_t tx=PE_LoadU32(target+0x28u),tz=PE_LoadU32(target+0x30u);
    if(x==tx && z==tz)return 1; /* Original retains the initialized task flag. */
    uint32_t speed=PE_LoadU32(actor+0x20u);
    if(actor==PE_LoadU32(0x8009D254u))speed=func_8003708C(0x50000u,speed);
    speed=func_8003708C(speed,(uint32_t)PE_LoadU16(actor+0x26u)<<4u);
    int32_t heading=(0x1400-func_80079FB4((int32_t)(z-tz),(int32_t)(x-tx)))&0xFFF;
    if(rate) {
        int32_t rot=(int16_t)PE_LoadU16(actor+0x3Au),desired=heading;
        if(rot<heading) {
            int32_t difference=heading-rot;
            if(difference<2048) {
                if(rate<difference)heading=(int32_t)((uint32_t)rot+(uint32_t)rate);
            } else if(rate<difference) {
                heading=(int32_t)((uint32_t)rot-(uint32_t)rate);
                if(heading<0 && rot+4096-desired<rate)heading=desired;
            }
        } else {
            int32_t difference=rot-heading;
            if(difference<2048) {
                if(rate<difference)heading=(int32_t)((uint32_t)rot-(uint32_t)rate);
            } else if(rate<difference) {
                heading=(int32_t)((uint32_t)rot+(uint32_t)rate);
                if(heading>4096 && desired+4096-rot<rate)heading=desired;
            }
        }
        heading&=0xFFF;PE_StoreU16(actor+0x3Au,(uint16_t)heading);
    }
    uint32_t vx=func_8003708C(0u-speed,(uint32_t)func_80077CF4(heading)<<4u);
    PE_StoreU32(actor+0x68u,vx);
    uint32_t vz=func_8003708C(0u-speed,(uint32_t)func_80077DC4(heading)<<4u);
    PE_StoreU32(actor+0x70u,vz);
    int32_t dx=(int32_t)(tx-x)>>16,dz=(int32_t)(tz-z)>>16;
    int32_t ix=(int16_t)(vx>>16),iz=(int32_t)vz>>16;
    uint32_t distance=(uint32_t)(dx*dx)+(uint32_t)(dz*dz);
    uint32_t movement=(uint32_t)(ix*ix)+(uint32_t)(iz*iz);
    if((int32_t)movement<(int32_t)distance) {
        PE_StoreU32(0x8009CE00u,PE_LoadU32(0x8009CE00u)-20u);
        PE_StoreU32(task+0x10u,1u);return 0;
    }
    PE_StoreU32(actor+0x28u,tx);PE_StoreU32(actor+0x30u,tz);
    PE_StoreU32(actor+0x68u,0);PE_StoreU32(actor+0x70u,0);
    PE_StoreU16(task+8u,(uint16_t)(PE_LoadU16(task+8u)&0xFFDFu));return 1;
}
