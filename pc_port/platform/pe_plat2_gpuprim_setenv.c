/* Environment packet builder 0x80075EE0 (called by PutDrawEnv 0x80075424 and DrawOTagEnv 0x800754E4), written
 * clean-room from the black-box vector set gpu2e (docs/evidence/native-gpu2-20261126; the generator spec
 * func_80075EE0 was added by the gpu2c author), the port's host GPU/DMA model API and psx-spx (GP0 E1h..E6h and the
 * 60h fill rectangle); no SDK source, decompiled source, tables or disassembly were read.
 *
 * Derivation (traced to the vectors, see AUTHOR-REPORT.md):
 *  - Inputs: the packet address and the environment record.  Environment halfwords (signed): clip x,y,w,h at +0,+2,
 *    +4,+6, offset x,y at +8,+0xA; texture-window rectangle at +0xC (passed by address); texture page halfword at +0x14;
 *    bytes at +0x16 (dither), +0x17 (draw-to-display), +0x18 (background flag), +0x19/+0x1A/+0x1B (red, green, blue).
 *  - Packet words (packet + 4 ...), each the result of a guest function reached by address: +4 = 0x80076170(x, y);
 *    +8 = 0x80076208(x + w - 1, y + h - 1); +0xC = 0x800762A0(offset x, offset y); +0x10 = 0x80076150(draw-to-display,
 *    dither, texture page); +0x14 = 0x800762BC(window rectangle address, ...); +0x18 = 0xE6000000.  The window call is
 *    made with the dither byte and texture page still in its second and third argument registers (significant in the
 *    vectors).
 *  - Background flag zero: the packet holds 6 words.  Otherwise three more words follow: +0x1C = command | red |
 *    green << 8 | blue << 16, +0x20 = position, +0x24 = (height << 16) | width (below).  When x and the clamped width are both
 *    multiples of 64 the command is 0x02000000 and the position is (y << 16) | (x & 0xFFFF) as stored; otherwise the
 *    command is 0x60000000 and the position is ((y - offset y) << 16) | ((x - offset x) & 0xFFFF).  The size is
 *    (height << 16) | width, where width (height) is 0 when the clip value is negative, else limit - 1 when it is not below the signed limit
 *    halfword at 0x80095750 (0x80095752), else unchanged; a limit is read only for a non-negative value.  The byte
 *    packet + 3 gets the word count (6 or 9), which is also the return value. */
#include <stdint.h>
#include "pe_guest_ram.h"
#include "pe_guestcode.h"
#include "pe_plat/gpuprim.h"

#define FN_AREA_TL   0x80076170u
#define FN_AREA_BR   0x80076208u
#define FN_OFFSET    0x800762A0u
#define FN_MODE      0x80076150u
#define FN_WINDOW    0x800762BCu
#define LIMIT_W      0x80095750u
#define LIMIT_H      0x80095752u
#define ENV_CLIP     0x00u
#define ENV_OFFSET   0x08u
#define ENV_WINDOW   0x0Cu
#define ENV_TPAGE    0x14u
#define ENV_DITHER   0x16u
#define ENV_DTD      0x17u
#define ENV_BG       0x18u
#define ENV_RGB      0x19u
#define WORD_MASK    0xE6000000u
#define FILL_RECT    0x60000000u
#define FILL_ALIGNED 0x02000000u

static int32_t half(uint32_t address) { return (int32_t)(int16_t)PE_LoadU16(address); }

/* clip size clamp: below zero is 0; not below the limit is limit - 1 (the limit is read only when needed) */
static int32_t clamp_size(int32_t value, uint32_t limit_address)
{
    if (value < 0) return 0;
    {
        int32_t limit = half(limit_address);
        if (value >= limit) return limit - 1;
    }
    return value;
}

static uint32_t call_word(uint32_t fn, uint32_t a0, uint32_t a1, uint32_t a2)
{
    return (uint32_t)PE_GuestCall("func_80075EE0", fn, 3, a0, a1, a2, 0);
}

int pe_plat_gpuprim_set_draw_env(uint32_t packet, uint32_t env)
{
    int32_t x = half(env + ENV_CLIP), y = half(env + ENV_CLIP + 2u), w = half(env + ENV_CLIP + 4u), h = half(env + ENV_CLIP + 6u);
    int32_t ox = half(env + ENV_OFFSET), oy = half(env + ENV_OFFSET + 2u);
    uint32_t dtd, dfe, tpage, count;
    PE_StoreU32(packet + 4u, call_word(FN_AREA_TL, (uint32_t)x, (uint32_t)y, 0));
    PE_StoreU32(packet + 8u, call_word(FN_AREA_BR, (uint32_t)(int32_t)(int16_t)(x + w - 1), (uint32_t)(int32_t)(int16_t)(y + h - 1), 0));
    PE_StoreU32(packet + 0xCu, call_word(FN_OFFSET, (uint32_t)ox, (uint32_t)oy, 0));
    dfe = PE_LoadU8(env + ENV_DTD);
    dtd = PE_LoadU8(env + ENV_DITHER);
    tpage = PE_LoadU16(env + ENV_TPAGE);
    PE_StoreU32(packet + 0x10u, call_word(FN_MODE, dfe, dtd, tpage));
    PE_StoreU32(packet + 0x14u, call_word(FN_WINDOW, env + ENV_WINDOW, dtd, tpage));
    PE_StoreU32(packet + 0x18u, WORD_MASK);
    if (PE_LoadU8(env + ENV_BG) == 0u) {
        count = 6u;
    } else {
        uint32_t red = PE_LoadU8(env + ENV_RGB), green = PE_LoadU8(env + ENV_RGB + 1u), blue = PE_LoadU8(env + ENV_RGB + 2u);
        uint32_t color = red | (green << 8) | (blue << 16);
        int32_t fw = clamp_size(w, LIMIT_W), fh = clamp_size(h, LIMIT_H);
        if ((x & 0x3F) == 0 && (fw & 0x3F) == 0) {          /* aligned: GP0 02h fill at the clip position */
            PE_StoreU32(packet + 0x1Cu, FILL_ALIGNED | color);
            PE_StoreU32(packet + 0x20u, ((uint32_t)y << 16) | ((uint32_t)x & 0xFFFFu));
        } else {                                            /* general: GP0 60h rectangle at clip - offset */
            PE_StoreU32(packet + 0x1Cu, FILL_RECT | color);
            PE_StoreU32(packet + 0x20u, ((uint32_t)(y - oy) << 16) | ((uint32_t)(x - ox) & 0xFFFFu));
        }
        PE_StoreU32(packet + 0x24u, ((uint32_t)fh << 16) | ((uint32_t)fw & 0xFFFFu));
        count = 9u;
    }
    PE_StoreU8(packet + 3u, (uint8_t)count);
    return (int)count;
}
