/* Isolated observation of current host InitGeom; no retail code or assets. */
#include <stdint.h>
#include <stdio.h>
#include "pe_guest_ram.h"
#include "game_port.h"
#include "pe_sdk.h"
void Trace_Direct(const char *s) { (void)s; }
int main(void)
{
    static const uint32_t expected[] = {0,0,1000,0xffffef9e,0x01400000,0x155,0x100};
    uint32_t random = 0x80077f7cu;
    for (unsigned test = 0; test < 24; ++test) {
        uint32_t before[32];
        PE_Port_RunControlReset(); PE_RamReset(); PE_Sdk_ResetState();
        PE_StoreU32(0x800a3450u, test % 3u);
        for (unsigned i = 0; i < 32; ++i) {
            random ^= random << 13; random ^= random >> 17; random ^= random << 5;
            PE_GTE_WriteCtrl(i, random); before[i] = PE_GTE_ReadCtrl(i);
        }
        func_80077F7C();
        if (PE_Port_ShouldStop() || PE_LoadU32(0x800a3450u) != test % 3u) return 1;
        for (unsigned i = 0; i < 32; ++i) {
            uint32_t want = (i >= 24 && i <= 30) ? expected[i - 24] : before[i];
            if (PE_GTE_ReadCtrl(i) != want) return 2;
        }
    }
    puts("PASS 24 host GTE initializations; guest callback word unchanged");
    return 0;
}
