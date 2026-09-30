/* M0000I original 80191854..80191C94: transition selection/setup.
 * Scene and encounter selectors preserve fields the original leaves untouched. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

void func_80191854(void)
{
    unsigned epoch=PE_Port_StopEpoch();
    func_8019BD78();
    if(PE_Port_StopEpoch()!=epoch)return;
    int active=func_8005BCB0();
    if(PE_Port_StopEpoch()!=epoch)return;
    PE_StoreU32(0x8019C008u,active?255u:0u);
    unsigned enabled=(PE_LoadU32(0x800B0CD8u)&0x40000000u)!=0;
    PE_StoreU32(0x8019C008u,enabled?255u:0u);
    func_800371A4((int)enabled);
    if(PE_Port_StopEpoch()!=epoch)return;

    uint32_t scene=PE_LoadU32(0x800A77F4u);
    uint16_t variant=7;
    switch(scene){
    case 0x179:case 0x18:variant=6;break;
    case 0x2E:case 0x75:case 0x67:variant=7;break;
    case 0xBF:case 0xC0:variant=9;break;
    case 0x5D:case 0x60:variant=2;break;
    case 0x3A:case 0x176:variant=8;break;
    case 0x7D:variant=4;break;
    case 0x104:variant=3;break;
    case 0xA1:variant=1;break;
    case 0xB7:variant=0;break;
    case 0x122:variant=5;break;
    default:break;
    }
    PE_StoreU16(0x8019CC52u,variant);
    uint32_t encounter=PE_LoadU32(0x800A7918u);
    switch(encounter){
    case 0x80:PE_StoreU32(0x8019BFF4u,0);break;
    case 0x208:PE_StoreU32(0x8019BFF4u,7);break;
    case 0xB8:case 0x148:
        PE_StoreU32(0x8019C000u,1);PE_StoreU32(0x8019C004u,9);break;
    case 0xD0:case 0x178:
        PE_StoreU32(0x8019C000u,1);PE_StoreU32(0x8019C004u,7);break;
    case 0xE0:
        PE_StoreU32(0x8019C000u,1);PE_StoreU32(0x8019C004u,8);break;
    case 0x1C0:
        PE_StoreU32(0x8019C000u,1);PE_StoreU32(0x8019C004u,4);break;
    case 0xC0:case 0x160:
        PE_StoreU32(0x8019BFF8u,1);PE_StoreU32(0x8019BFFCu,9);break;
    case 0xD8:case 0x180:
        PE_StoreU32(0x8019BFF8u,1);PE_StoreU32(0x8019BFFCu,7);break;
    case 0xE4:
        PE_StoreU32(0x8019BFF8u,1);PE_StoreU32(0x8019BFFCu,8);break;
    case 0x1C8:
        PE_StoreU32(0x8019BFF8u,1);PE_StoreU32(0x8019BFFCu,4);break;
    default:break;
    }
    PE_StoreU8(0x8019C1F0u,(PE_LoadU32(0x800A77FCu)&0x2000u)?1:0);
    func_80191C94();
}
