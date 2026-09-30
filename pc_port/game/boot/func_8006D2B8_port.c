#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

#define OVERLAY 0x800B0CD8u
#define RECORD 0x8009D180u
#define BANK 0x8009D184u

/* DAY1-2: complete original 8006D2B8..8006D60C music-bank loader.
 * CD/SPU work remains delegated to the original loader's host providers. */
int func_8006D2B8(int id, int load, int second, pe_addr_t out_slot, int blocking)
{
    uint32_t package=PE_LoadU32(OVERLAY+0x18Cu);
    uint32_t header=package+PE_LoadU32(package+4u);
    unsigned slot=(uint32_t)second!=0;
    for (;;) {
        switch (PE_LoadU8(OVERLAY+0xF1u)) {
        case 0: {
            uint32_t packed=PE_LoadU32(header+0x30u);
            uint32_t record=PE_LoadU32(OVERLAY+0x18Cu)+(packed&0x3FFFFFu);
            PE_StoreU16(BANK,0xFFFFu);
            PE_StoreU32(RECORD,record);
            for (unsigned i=0;i<(packed>>22);i++,record+=12u) {
                if ((int)PE_LoadU16(record+10u)==id) {
                    PE_StoreU32(RECORD,record);
                    PE_StoreU16(BANK,PE_LoadU16(record+8u));
                    break;
                }
            }
            if ((int16_t)PE_LoadU16(BANK)==-1) {
                for (unsigned i=0;i<2;i++) {
                    if ((int8_t)PE_LoadU8(OVERLAY+0xDCu+2u*i)==id) {
                        PE_StoreU32(RECORD,0);
                        PE_StoreU16(BANK,(uint16_t)(int16_t)(int8_t)PE_LoadU8(OVERLAY+0xDAu+i));
                        break;
                    }
                }
                if ((int16_t)PE_LoadU16(BANK)==-1) {
                    PE_StoreU32(out_slot,0xFFFFFFFFu);
                    return 0;
                }
            }
            if (load) {
                for (unsigned i=0;i<2;i++) {
                    if ((int8_t)PE_LoadU8(OVERLAY+0xDCu+2u*i)==id) {
                        PE_StoreU32(out_slot,(PE_LoadU32(OVERLAY)&(0x40u<<i))?0xFFFFFFFEu:i);
                        return 0;
                    }
                }
                if (!second) func_80086FF8();
                PE_StoreU8(OVERLAY+0xF1u,7);
                continue;
            }
            for (unsigned i=0;i<2;i++) {
                if ((int8_t)PE_LoadU8(OVERLAY+0xDCu+2u*i)==id) {
                    PE_StoreU8(OVERLAY+0xDCu+2u*i,0xFF);
                    PE_StoreU8(OVERLAY+0xDAu+i,0xFF);
                    PE_StoreU32(OVERLAY,PE_LoadU32(OVERLAY)&~(0x40u<<i));
                    PE_StoreU32(out_slot,i);
                }
            }
            return 0;
        }
        case 7:
            if (func_8006CDA4(0,(int16_t)PE_LoadU16(BANK),0,
                    PE_LoadU32(OVERLAY+0x194u),0x21,blocking)==1) {
                if (!(blocking&1) || PE_Port_ShouldStop()) return 1;
            } else PE_StoreU8(OVERLAY+0xF1u,9);
            continue;
        case 9: {
            PE_StoreU32(out_slot,slot);
            uint32_t record=PE_LoadU32(RECORD);
            if (record) {
                uint32_t src=PE_LoadU32(OVERLAY+0x18Cu)+(PE_LoadU32(record+4u)&0xFFFFFFu);
                uint32_t count=PE_LoadU32(record)&0xFFFFFFu;
                uint32_t dst=PE_LoadU32(OVERLAY+0x128u+4u*slot);
                /* Original 71A34 invokes BIOS A0 memcpy (service 2A). */
                for (uint32_t i=0;i<count;i++) PE_StoreU8(dst+i,PE_LoadU8(src+i));
            }
            uint32_t selected=PE_LoadU32(out_slot);
            PE_StoreU8(OVERLAY+0xDCu+2u*selected,(uint8_t)id);
            selected=PE_LoadU32(out_slot);
            PE_StoreU8(OVERLAY+0xDAu+selected,(uint8_t)PE_LoadU16(BANK));
            PE_StoreU8(OVERLAY+0xF1u,0);
            return 0;
        }
        default:
            /* Retail spins in unused states; retain an explicit boundary. */
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 1;
        }
    }
}
