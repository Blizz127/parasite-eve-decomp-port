/* Independent 80080220 retail core from original-code black-box vectors.
 * Known table targets execute native C; unknown targets preserve the host stop. */
#include "pe_guest_ram.h"
#include "pe_sdk.h"
#include "game_port.h"
#include "pe_bootstrap.h"
static void idle_state(void)
{ PE_StoreU32(0x8009B574u,1u);PE_StoreU32(0x8009B578u,11u); }
void pe_plat_cd_command_complete(uint32_t status,pe_addr_t response)
{
    if (PE_Port_ShouldStop()) return;
    status &= 255u;
    if (status == 2u) {
        uint8_t command = PE_LoadU8(0x8009B558u);
        if (command == 3u || command == 6u || command == 27u) {
            PE_StoreU32(0x8009B578u,command == 3u ? 16u : 17u);
            PE_StoreU32(0x8009B574u,2u);
            PE_StoreU8(0x8009B587u,command);
            PE_StoreU32(0x8009B5A0u,1200u);
        } else if (command == 14u) {
            uint8_t mode = PE_LoadU8(0x8009B559u);
            if ((PE_LoadU8(0x8009B581u) ^ mode) & 0x80u) {
                PE_StoreU32(0x8009B578u,15u);
                PE_StoreU32(0x8009B574u,2u);
                PE_StoreU32(0x8009B594u,3u);
            } else idle_state();
            PE_StoreU8(0x8009B581u,mode);
        } else {
            uint32_t target=0x80080368u;
            if (command >= 2u && command <= 26u)
                target=PE_LoadU32(0x80011D04u+4u*command);
            if (target == 0x80080318u) {
                uint32_t first=PE_LoadU32(0x8009B558u),last=PE_LoadU32(0x8009B55Cu);
                PE_StoreU32(0x8009B584u,(PE_LoadU32(0x8009B584u)&0xffff0000u)|(first>>24)|((last&255u)<<8));
                PE_StoreU32(0x8009B580u,(PE_LoadU32(0x8009B580u)&0xffffu)|((first&0x00ffff00u)<<8));
            } else if (target == 0x80080340u) {
                PE_StoreU8(0x8009B586u,command);
            } else if (target != 0x80080368u) {
                Bootstrap_ReturnVoid4Indirect("CD_command_table","func_80080220",target,status,response,0u,0u);
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                return;
            }
            idle_state();
        }
    } else if (PE_LoadU8(0x8009B56Cu)&0x10u) {
        PE_StoreU32(0x8009B574u,2u);
        PE_StoreU32(0x8009B578u,12u);
    } else idle_state();
    if (PE_LoadU32(0x800A36A4u) && PE_LoadU32(0x8009B554u))
        (void)PE_Cd_DispatchDataCallback(PE_LoadU32(0x800A36A4u),status,response);
}
