/*
 * Phase 6E-B54K-T — func_8018F2F4 title-bank framebuffer present.
 *
 * Retail overlay: [0x8018F2F4, 0x8018F468), 93 words / 0x174.
 * Flips D_801D11C8, selects DRAWENV, LoadImage of the title strip
 * (0, y=20|260, 480x204) from overlay offset *D_80193258 + 0x80193268
 * (= record+0x14 for the TIM at anchor+0x28), optional env+0x8080 blit,
 * then VSync / ResetGraph / PutDrawEnv / PutDispEnv. Loops twice.
 */
#include "psx_compat.h"
#include "game_port.h"

#define GA_ENVIRONMENT_0        0x801D11BCu
#define GA_ENVIRONMENT_CURRENT  0x801D11C4u
#define GA_ENVIRONMENT_TOGGLE   0x801D11C8u
#define GA_TITLE_STRIP_OFFSET   0x80193258u
#define GA_TITLE_STRIP_PIXELS   0x80193268u

void func_8018F2F4(void)
{
    unsigned bank;
    pe_addr_t environment;
    pe_addr_t pixels;
    RECT rect;
    unsigned i;

    for (i = 0u; i < 2u; i++) {
        bank = PE_LoadU32(GA_ENVIRONMENT_TOGGLE) == 0u ? 1u : 0u;
        environment = PE_LoadU32(GA_ENVIRONMENT_0 + bank * 4u);
        PE_StoreU32(GA_ENVIRONMENT_TOGGLE, bank);
        PE_StoreU32(GA_ENVIRONMENT_CURRENT, environment);

        rect.x = 0;
        rect.y = (int16_t)(bank != 0u ? 20 : 260);
        rect.w = 480;
        rect.h = 204;
        pixels = PE_LoadU32(GA_TITLE_STRIP_OFFSET) + GA_TITLE_STRIP_PIXELS;
        (void)func_8007506C(&rect, pixels);
        if (PE_Port_ShouldStop())
            return;
        (void)func_80074DC0(0);
        if (PE_Port_ShouldStop())
            return;

        if ((int16_t)PE_LoadU16(environment + 0x74u) > 0) {
            RECT optional;
            int32_t w;
            int32_t h;

            optional.x = (int16_t)PE_LoadU16(environment + 0x70u);
            optional.y = (int16_t)PE_LoadU16(environment + 0x72u);
            optional.w = (int16_t)PE_LoadU16(environment + 0x74u);
            optional.h = (int16_t)PE_LoadU16(environment + 0x76u);
            w = (int32_t)optional.x;
            optional.x = (int16_t)((w + w + w) >> 1);
            if (PE_LoadU32(GA_ENVIRONMENT_TOGGLE) == 0u)
                optional.y = (int16_t)(optional.y + 240);
            h = (int32_t)optional.w;
            optional.w = (int16_t)((h + h + h) >> 1);
            (void)func_8007506C(&optional, environment + 0x8080u);
            if (PE_Port_ShouldStop())
                return;
        }

        (void)func_80073A44(0);
        (void)func_80074A44(1);
        (void)func_80075424(environment);
        if (PE_Port_ShouldStop())
            return;
        (void)func_800755F0(environment + 0x5Cu);
        if (PE_Port_ShouldStop())
            return;
    }
}
