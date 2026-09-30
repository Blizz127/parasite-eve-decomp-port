/*
 * Phase 6E-B54K-X — complete func_80190660 fade/display initializer.
 *
 * Retail function: [0x80190660,0x801909B4), 0x354 / 213 words.
 * Translated body: [0x80190660,0x801909B4), 0x354 / 213 words.
 *
 * The two caller-stack packet banks remain native transients. No native
 * pointer is retained as guest authority.
 */
#include "psx_compat.h"
#include "game_port.h"

#include <string.h>

#define GA_OVERLAY_DATA_OFFSET  0x80193278u
#define GA_OVERLAY_DATA_ANCHOR  0x80193254u
#define GA_ENVIRONMENT_0        0x801D11BCu
#define GA_ENVIRONMENT_CURRENT  0x801D11C4u
#define GA_ENVIRONMENT_TOGGLE   0x801D11C8u

static RECT PE_OverlayLoadRect(pe_addr_t address)
{
    RECT rect;

    rect.x = (int16_t)PE_LoadU16(address + 0u);
    rect.y = (int16_t)PE_LoadU16(address + 2u);
    rect.w = (int16_t)PE_LoadU16(address + 4u);
    rect.h = (int16_t)PE_LoadU16(address + 6u);
    return rect;
}

static void PE_TransientStoreU16(uint8_t *destination, uint16_t value)
{
    destination[0] = (uint8_t)value;
    destination[1] = (uint8_t)(value >> 8);
}

static void PE_TransientStoreU32(uint8_t *destination, uint32_t value)
{
    destination[0] = (uint8_t)value;
    destination[1] = (uint8_t)(value >> 8);
    destination[2] = (uint8_t)(value >> 16);
    destination[3] = (uint8_t)(value >> 24);
}

static uint32_t PE_TransientLoadU32(const uint8_t *source)
{
    return (uint32_t)source[0] |
           ((uint32_t)source[1] << 8) |
           ((uint32_t)source[2] << 16) |
           ((uint32_t)source[3] << 24);
}

static void PE_OverlayBuildSprtPair(uint8_t *pair)
{
    uint8_t *second = pair + 0x14u;

    pair[3] = 4u;
    pair[7] = 0x64u;
    PE_TransientStoreU16(pair + 8u, 32u);
    PE_TransientStoreU16(pair + 10u, 88u);
    pair[12] = 0u;
    pair[13] = 0u;
    PE_TransientStoreU16(pair + 14u, 0x7800u);
    PE_TransientStoreU16(pair + 16u, 256u);
    PE_TransientStoreU16(pair + 18u, 64u);

    second[3] = 4u;
    second[7] = 0x64u;
    PE_TransientStoreU16(second + 8u, 284u);
    PE_TransientStoreU16(second + 10u, 80u);
    second[12] = 0u;
    second[13] = 0u;
    PE_TransientStoreU16(second + 14u, 0x7800u);
    PE_TransientStoreU16(second + 16u, 8u);
    PE_TransientStoreU16(second + 18u, 80u);
}

static void PE_OverlayBuildDrawModePair(uint8_t *pair)
{
    pair[3] = 1u;
    PE_TransientStoreU32(pair + 4u, 0xE1000018u);
    pair[11] = 1u;
    PE_TransientStoreU32(pair + 12u, 0xE1000019u);
}

int func_80190660(void)
{
    uint8_t sprites[0x50];
    uint8_t draw_modes[0x20];
    pe_addr_t record;
    pe_addr_t second;
    pe_addr_t environment;
    uint32_t next_toggle;
    uint32_t intensity;
    uint32_t draw_mode_command;
    uint32_t sprite_words[4];
    uint32_t frame = 0u;
    RECT rect;
    unsigned parity;

    /* 0x80190698..0x801906DC: two overlay-resident image records. */
    record = GA_OVERLAY_DATA_ANCHOR + PE_LoadU32(GA_OVERLAY_DATA_OFFSET);
    rect = PE_OverlayLoadRect(record + 0x0Cu);
    (void)func_8007506C(&rect, record + 0x14u);
    if (PE_Port_ShouldStop())
        return -1;

    second = record + 8u + (PE_LoadU32(record + 8u) & ~3u);
    rect = PE_OverlayLoadRect(second + 4u);
    (void)func_8007506C(&rect, second + 0x0Cu);
    if (PE_Port_ShouldStop())
        return -1;
    if (func_80074DC0(0) != 0 || PE_Port_ShouldStop())
        return -1;

    /* 0x801906E8..0x8019079C: two parity banks of DR_MODE + SPRT pairs. */
    memset(sprites, 0, sizeof(sprites));
    memset(draw_modes, 0, sizeof(draw_modes));
    for (parity = 0u; parity < 2u; parity++) {
        PE_OverlayBuildDrawModePair(draw_modes + parity * 0x10u);
        PE_OverlayBuildSprtPair(sprites + parity * 0x28u);
    }

    /* Both display environments are disabled while the fade starts. */
    PE_StoreU8(PE_LoadU32(GA_ENVIRONMENT_0 + 4u) + 0x6Du, 0u);
    PE_StoreU8(PE_LoadU32(GA_ENVIRONMENT_0) + 0x6Du, 0u);
    func_80074D28(1);

    /* Complete 480-frame loop at 0x801907C4..0x8019095C. */
    while (frame < 480u) {
    parity = frame & 1u;
    next_toggle = PE_LoadU32(GA_ENVIRONMENT_TOGGLE) == 0u ? 1u : 0u;
    environment = PE_LoadU32(GA_ENVIRONMENT_0 + next_toggle * 4u);
    PE_StoreU32(GA_ENVIRONMENT_TOGGLE, next_toggle);
    PE_StoreU32(GA_ENVIRONMENT_CURRENT, environment);

    if (frame < 32u)
        intensity = frame * 4u;
    else if (frame < 392u)
        intensity = 128u;
    else if (frame < 424u)
        intensity = (424u - frame) * 4u;
    else
        intensity = 0u;

    sprites[parity * 0x28u + 4u] = (uint8_t)intensity;
    sprites[parity * 0x28u + 5u] = (uint8_t)intensity;
    sprites[parity * 0x28u + 6u] = (uint8_t)intensity;

    /* B54K-U: the first DrawPrim now traverses the exact wrapper/worker and
     * generic GP0(E1h) draw-mode state. */
    draw_mode_command = 0xE1000018u;
    if (PE_func_80075358_Transient(&draw_mode_command, 1u) != 0 ||
        PE_Port_ShouldStop())
        return -1;

    /* B54K-V: the four initialized SPRT words traverse the same synchronous
     * DrawPrim wrapper. No native stack pointer is retained. */
    for (unsigned i = 0u; i < 4u; i++)
        sprite_words[i] = PE_TransientLoadU32(
            sprites + parity * 0x28u + 4u + i * 4u);
    if (PE_func_80075358_Transient(sprite_words, 4u) != 0 ||
        PE_Port_ShouldStop())
        return -1;

    /* Retail advances to the paired sprite, writes the same frame intensity
     * into RGB, then performs the explicit DrawSync. */
    sprites[parity * 0x28u + 0x18u] = (uint8_t)intensity;
    sprites[parity * 0x28u + 0x19u] = (uint8_t)intensity;
    sprites[parity * 0x28u + 0x1Au] = (uint8_t)intensity;
    if (func_80074DC0(0) != 0 || PE_Port_ShouldStop())
        return -1;

    /* The canonical environments have width zero and bypass this upload.
     * Preserve the complete retail-positive path rather than specializing
     * that state. */
    if ((int16_t)PE_LoadU16(environment + 0x74u) > 0) {
        RECT optional;

        optional.x = (int16_t)(((int32_t)(int16_t)
            PE_LoadU16(environment + 0x70u) * 3) >> 1);
        optional.y = (int16_t)PE_LoadU16(environment + 0x72u);
        optional.w = (int16_t)(((int32_t)(int16_t)
            PE_LoadU16(environment + 0x74u) * 3) >> 1);
        optional.h = (int16_t)PE_LoadU16(environment + 0x76u);
        if (PE_LoadU32(GA_ENVIRONMENT_TOGGLE) == 0u)
            optional.y = (int16_t)(optional.y + 240);
        (void)func_8007506C(&optional, environment + 0x8080u);
        if (PE_Port_ShouldStop())
            return -1;
    }

    func_80073A44(0);
    (void)func_80074A44(1);

    /* 0x8019093C..0x80190950: PutDrawEnv and PutDispEnv.  Frame increments
     * in the first call's delay slot. */
    frame++;
    (void)func_80075424(environment);
    if (PE_Port_ShouldStop())
        return -1;
    func_800755F0(environment + 0x5Cu);
    if (PE_Port_ShouldStop())return -1;

    }

    /* 0x80190960..0x801909B0: hide display, re-enable both environment
     * display flags, and return normally. */
    func_80074D28(0);
    PE_StoreU8(PE_LoadU32(GA_ENVIRONMENT_0 + 4u) + 0x6Du, 1u);
    PE_StoreU8(PE_LoadU32(GA_ENVIRONMENT_0) + 0x6Du, 1u);
    return 0;
}
