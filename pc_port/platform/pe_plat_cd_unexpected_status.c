/* Independent 8008068C draft from black-box vectors. Host callback dispatch retains explicit unsupported-target stops. */
#include "pe_guest_ram.h"
#include "pe_sdk.h"
#include "game_port.h"
void pe_plat_cd_unexpected_status(uint32_t status, pe_addr_t response)
{
    if (PE_Port_ShouldStop()) return;
    status &= 255u;
    if (status == 2u) {
        if (PE_LoadU8(0x8009B558u) == 14u) {
            uint8_t old = PE_LoadU8(0x8009B581u);
            uint8_t mode = PE_LoadU8(0x8009B559u);
            if ((old ^ mode) & 0x80u) {
                PE_StoreU32(0x8009B578u, 15u);
                PE_StoreU32(0x8009B574u, 2u);
                PE_StoreU32(0x8009B594u, 3u);
            }
            PE_StoreU8(0x8009B581u, mode);
        }
    } else if (status == 5u) {
        if (PE_LoadU8(0x8009B56Cu) & 0x10u) {
            PE_StoreU32(0x8009B574u, 2u);
            PE_StoreU32(0x8009B578u, 12u);
            if (PE_LoadU32(0x800A36A4u) && PE_LoadU32(0x8009B554u))
                (void)PE_Cd_DispatchDataCallback(PE_LoadU32(0x800A36A4u), status, response);
        } else {
            PE_StoreU32(0x8009B574u, 1u);
            PE_StoreU32(0x8009B578u, 11u);
        }
    }
}
