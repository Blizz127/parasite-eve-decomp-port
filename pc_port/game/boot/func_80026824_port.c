/* Original battle item / PE / equipment menu command completion, 120D8.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

static void item_command_sound(uint32_t id)
{
    pe_addr_t package=PE_LoadU32(0x800B0E08u);
    if (package) func_8006DF50(package,id,0u,128u,127u);
}
static void append_item_command(pe_addr_t target,uint16_t command)
{
    uint32_t count=PE_LoadU8(0x8009CE3Cu);
    pe_addr_t slot=0x800BE830u+count*8u;
    PE_StoreU32(slot,target);PE_StoreU16(slot+4u,command);
    PE_StoreU16(slot+6u,(uint16_t)(int16_t)(int8_t)PE_LoadU8(0x8009D2D8u));
    PE_StoreU8(0x8009CE3Cu,(uint8_t)(count+1u));
}

int func_80026824(int initial_mode)
{
    int8_t mode=(int8_t)initial_mode;
    int32_t command;
    if (mode==1) {
        command=(int16_t)PE_LoadU16(0x8009D2A4u);PE_StoreU8(0x8009CE60u,0u);
        if (command<=0) return command==-1?0:mode;
        if (command==393 || command==394 || command==395 || command==397) {
            func_80026FD0();PE_StoreU8(0x8009CE40u,(uint8_t)(command==397?7:command-389));mode=2;
        } else if (command==406) {
            unsigned i;uint8_t actions=0u;
            func_80026FD0();PE_StoreU8(0x8009D25Cu,0u);
            for (i=0;i<7;i++) {
                int32_t random=(int32_t)func_80071A54(),count=(int8_t)PE_LoadU8(0x8009D2B0u);
                pe_addr_t target;
                if (!count) {
                    Bootstrap_ReturnVoid("func_80026824_target_divide","func_80026824");
                    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
                }
                actions=PE_LoadU8(0x8009D2D8u);
                target=PE_LoadU32(0x8009E000u+(uint32_t)(random%count)*12u);
                append_item_command(target,PE_LoadU16(0x8009D2A4u));
            }
            PE_StoreU8(0x8009D2D8u,(uint8_t)(actions-1u));PE_StoreU8(0x8009CE40u,0u);mode=0;
        } else if (command<407) {
            PE_StoreU8(0x8009CE40u,0u);
            append_item_command(PE_LoadU32(0x8009D254u),(uint16_t)command);
            PE_StoreU8(0x8009D2D8u,(uint8_t)(PE_LoadU8(0x8009D2D8u)-1u));mode=0;
        } else if (command<409) {
            uint32_t count=PE_LoadU8(0x8009CE3Cu);
            pe_addr_t slot=0x800BE830u+count*8u;
            PE_StoreU32(slot,PE_LoadU32(0x8009D254u));PE_StoreU16(slot+4u,(uint16_t)command);
            PE_StoreU16(slot+6u,(uint16_t)(int16_t)(int8_t)PE_LoadU8(0x8009D2D8u));
            func_800254BC(command);
            PE_StoreU8(0x8009D2D8u,0u);PE_StoreU8(0x8009CE40u,0u);
            PE_StoreU8(0x8009CE3Cu,(uint8_t)(PE_LoadU8(0x8009CE3Cu)+1u));mode=-1;
        } else {
            append_item_command(PE_LoadU32(0x8009D254u),(uint16_t)command);
            PE_StoreU8(0x8009D2D8u,0u);PE_StoreU8(0x8009CE40u,0u);mode=-1;
        }
        if (!(int8_t)PE_LoadU8(0x8009CE40u)) item_command_sound(0x44Cu);
        PE_StoreU16(0x8009CE50u,PE_LoadU16(0x8009D2A4u));return mode;
    }
    if (mode==2) {
        func_800258CC((int8_t)PE_LoadU8(0x8009CE40u));
        if (PE_LoadU32(0x8009D1F4u)&0x200u) {
            pe_addr_t target=PE_LoadU32(0x8009E000u+(uint32_t)(int32_t)(int8_t)PE_LoadU8(0x8009CE44u)*12u);
            PE_StoreU8(0x8009CE40u,0u);append_item_command(target,PE_LoadU16(0x8009CE50u));
            PE_StoreU8(0x8009D2D8u,(uint8_t)(PE_LoadU8(0x8009D2D8u)-1u));item_command_sound(0x44Cu);return 0;
        }
        if (PE_LoadU32(0x8009D1F4u)&0x400u) {
            func_8005112C();PE_StoreU8(0x8009CE44u,0u);item_command_sound(0x44Du);return 0;
        }
    }
    return mode;
}
