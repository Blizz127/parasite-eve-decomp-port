/* Audio command RAM semantics, transcribed from disc 1 7A510/7B39C/
 * 7B9FC/7CA90/7CF20/7D284/7E044 and the matching src/ leaves.
 *
 * 8CA84 is the consumer of the 36-byte commands at B8628. The producer
 * does not wrap: the timer normally drains these between game frames.
 * Leaving it unserviced eventually overwrites Aya's B8A20 battle record.
 *
 * This supplies command dispatch and voice allocation/control. Score
 * interpretation (8DB7C's voice ticks) and audible SPU synthesis remain
 * separate, unported work. The host event service below invokes only the
 * command portion of that timer, and does not claim the full callback.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

static void audio_or(pe_addr_t address, uint32_t bits)
{
    PE_StoreU32(address, PE_LoadU32(address) | bits);
}

static void audio_and(pe_addr_t address, uint32_t bits)
{
    PE_StoreU32(address, PE_LoadU32(address) & bits);
}

/* 89960, 89B28 and 89CF0 each mark the same pending-update bit. */
static void audio_dirty(void) { audio_or(0x8009D2C4u, 0x100u); }

/* 8F1B0 removes a sound channel from all active/update masks. */
static void audio_release(pe_addr_t voice, uint32_t bit)
{
    static const pe_addr_t masks[] = {
        0x800BCD50u, 0x800BCD6Cu, 0x800BCD70u, 0x800BCD74u, 0x800BCD54u
    };
    unsigned i;
    for (i = 0; i < sizeof(masks)/sizeof(masks[0]); i++) audio_and(masks[i], ~bit);
    PE_StoreU32(voice + 0x2Cu, 0u);
    PE_StoreU32(voice + 0x28u, 0u);
}

static void audio_stop_voice(pe_addr_t voice, uint32_t bit)
{
    if (PE_LoadU32(voice + 0x38u) & 0x100000u) {
        audio_or(voice + 0x38u, 0x200000u);
    } else {
        audio_or(0x800BCD5Cu, bit);
        audio_release(voice, bit);
        PE_StoreU32(voice + 0x38u, 0u);
    }
}

/* Complete 8A400: group, handle, adjacent pair and oldest-voice stop. */
static void audio_stop(uint32_t handle, uint32_t group)
{
    uint32_t active = PE_LoadU32(0x800BCD50u), bit;
    unsigned i;
    int32_t oldest = 0;
    handle &= 0xFFFFu;
    if (handle == 0xFFFFu) return;
    if (!(group & 0x0FFFFFFFu) && (int32_t)group < 0) {
        pe_addr_t voice = 0x800BC000u + handle * 0x11Cu;
        bit = 0x1000u << (handle & 31u);
        if (active & bit) audio_stop(PE_LoadU32(voice + 0x28u), 0u);
        bit <<= 1;
        if (active & bit) audio_stop(PE_LoadU32(voice + 0x144u), 0u);
        return;
    }
    if (!(group & 0x0FFFFFFFu) && (group & 0x40000000u)) {
        for (i = 0, bit = 0x1000u; i < 12; i++, bit <<= 1) {
            pe_addr_t voice = 0x800BC000u + i * 0x11Cu;
            if (PE_LoadU32(voice + 0x2Cu)) active &= ~bit;
        }
        for (i = 0, bit = 0x1000u; i < 12; i++, bit <<= 1) {
            int32_t age = (int32_t)PE_LoadU32(0x800BC050u + i * 0x11Cu);
            if ((active & bit) && age > oldest) oldest = age;
        }
    }
    for (i = 0, bit = 0x1000u; i < 12; i++, bit <<= 1) {
        pe_addr_t voice = 0x800BC000u + i * 0x11Cu;
        if (!(active & bit)) continue;
        if (group & 0x0FFFFFFFu) {
            if (!(PE_LoadU32(voice + 0x2Cu) & group)) continue;
        } else if (group & 0x40000000u) {
            if ((int32_t)PE_LoadU32(voice + 0x50u) != oldest) continue;
        } else if (PE_LoadU32(voice + 0x28u) != handle) continue;
        audio_stop_voice(voice, bit);
    }
    audio_or(0x8009D2C4u, 0x10u);
    audio_dirty();
}

static void audio_instrument_init(pe_addr_t voice)
{
    unsigned i;
    PE_StoreU16(voice + 0x5Au, 0u);
    PE_StoreU32(voice + 0xF8u, PE_LoadU32(0x800B2900u));
    PE_StoreU32(voice + 0xFCu, PE_LoadU32(0x800B2904u));
    for (i = 0; i < 4; i++)
        PE_StoreU16(voice + 0x10Eu + i * 2u, PE_LoadU8(0x800B2908u + i));
    PE_StoreU32(voice + 0x100u, PE_LoadU8(0x800B290Du));
    PE_StoreU32(voice + 0x104u, PE_LoadU8(0x800B290Eu));
    PE_StoreU16(voice + 0x116u, PE_LoadU8(0x800B290Cu));
    audio_or(voice + 0xF4u, 0x1FF80u);
    PE_StoreU32(voice + 0x108u, PE_LoadU8(0x800B290Fu));
}

/* 89F58 -> 8F178(0) -> 8F0D0: initialize the selected instrument. */
static void audio_voice_init(pe_addr_t voice, pe_addr_t score)
{
    static const uint16_t zero_fields[] = {
        0xDE,0xE0,0x82,0xE4,0x7A,0xD2,0xD0,0x72,0xCE,0xEC,
        0x84,0xB4,0xA6,0x94,0xB6,0xA8,0x96,0xBC,0xBA
    };
    unsigned i;
    PE_StoreU16(voice + 0x6Cu, 0x6E00u);
    PE_StoreU32(voice, score);
    for (i = 0; i < sizeof(zero_fields)/sizeof(zero_fields[0]); i++)
        PE_StoreU16(voice + zero_fields[i], 0u);
    PE_StoreU32(voice + 0x34u, 0u);
    PE_StoreU32(voice + 0x44u, 0x32000000u);
    PE_StoreU32(voice + 0x38u, 0u);
    audio_instrument_init(voice);
}

/* 8A750: publish one sound voice, including pause-mask behavior. */
static void audio_start_voice(pe_addr_t voice, pe_addr_t entry,
                              uint32_t bit, pe_addr_t score)
{
    static const pe_addr_t clear_masks[] = {
        0x800BCD54u,0x800BCD58u,0x800BCD6Cu,0x800BCD70u,0x800BCD74u
    };
    unsigned i;
    PE_StoreU32(voice + 0x28u, PE_LoadU32(entry + 4u));
    PE_StoreU32(voice + 0x2Cu, PE_LoadU32(entry + 8u));
    PE_StoreU16(voice + 0x78u, 0u);
    PE_StoreU16(voice + 0x76u, (uint16_t)(PE_LoadU8(entry + 12u) << 8));
    PE_StoreU16(voice + 0x56u, 2u);
    PE_StoreU16(voice + 0x58u, 1u);
    PE_StoreU16(voice + 0x54u, 1u);
    PE_StoreU16(voice + 0x74u, 0u);
    PE_StoreU32(voice + 0x50u, 0xFFFFFFFEu);
    PE_StoreU16(voice + 0xD8u, (uint16_t)((PE_LoadU32(entry + 16u) & 127u) << 8));
    audio_voice_init(voice, score);
    audio_or(0x800BCD50u, bit);
    audio_or(0x800BCD5Cu, bit);
    for (i = 0; i < sizeof(clear_masks)/sizeof(clear_masks[0]); i++)
        audio_and(clear_masks[i], ~bit);
    if (PE_LoadU32(0x8009D2DCu) & 2u) {
        for (i = 0, bit = 0x1000u; i < 12; i++, bit <<= 1) {
            if (!(PE_LoadU32(0x800BC02Cu + i * 0x11Cu) & 0x2000000u)) {
                audio_and(0x800BCD50u, ~bit);
                audio_or(0x800BCD60u, bit);
            }
        }
    }
}

/* 8A8CC: an effect taking a physical voice detaches its music owner. */
static void audio_detach_music(uint32_t channel)
{
    unsigned i;
    if (channel >= 24u) return;
    for (i = 0; i < 24; i++) {
        pe_addr_t slot = 0x800B8BB0u + i * 0x11Cu;
        if (PE_LoadU32(slot) == channel) {
            PE_StoreU32(slot, 24u);
            audio_and(PE_LoadU32(0x8009D2C8u) + 0x14u, ~(1u << i));
        }
    }
}

/* 8A92C, including the original retry when an oldest voice is reclaimed. */
static void audio_start(pe_addr_t entry, pe_addr_t first, pe_addr_t second)
{
    pe_addr_t voice;
    uint32_t active, bit;
    unsigned remaining;
    if (!first && !second) return;
    if (PE_LoadU32(entry + 8u)) audio_stop(0u, PE_LoadU32(entry + 8u));
    for (;;) {
        active = PE_LoadU32(0x800BCD50u);
        voice = 0x800BCC34u;
        bit = 0x800000u;
        remaining = 12u;
        if (first && second) { voice -= 0x11Cu; bit >>= 1; remaining--; }
        while (active & (bit | ((first && second) ? bit << 1 : 0u))) {
            voice -= 0x11Cu; bit >>= 1;
            if (!--remaining) break;
        }
        if (remaining) break;
        audio_stop(0u, 0x40000000u);
        if (active == PE_LoadU32(0x800BCD50u)) return;
    }
    if ((int32_t)active < 0) return;
    if (first) {
        audio_start_voice(voice, entry, bit, first);
        audio_detach_music(PE_LoadU32(voice + 0xF0u));
    }
    if (second) {
        if (first) { voice += 0x11Cu; bit <<= 1; }
        audio_start_voice(voice, entry, bit, second);
        audio_detach_music(PE_LoadU32(voice + 0xF0u));
        if (first) audio_or(voice + 0x38u, 0x10000u);
    }
    audio_or(0x8009D2C4u, 0x10u);
    audio_dirty();
}

/* 8A354: a0==0 stops the current bank when state+4 is live; nonzero
 * a0 stops only when it equals the bank's song id at state+0x54. */
static uint32_t audio_music_physical_mask(pe_addr_t voices, uint32_t selected)
{
    uint32_t mask=0;
    for(unsigned i=0;i<24;i++) {
        if(selected&(1u<<i)) {
            uint32_t channel=PE_LoadU32(voices+i*0x11Cu+0xF0u);
            if(channel<24u)mask|=1u<<channel;
        }
    }
    return mask;
}

static void audio_copy(pe_addr_t source, pe_addr_t dest, unsigned size)
{
    for(unsigned i=0;i<size;i++)PE_StoreU8(dest+i,PE_LoadU8(source+i));
}

/* Complete original 8A068: initialize all24 primary music voices. */
static void audio_music_new(pe_addr_t payload)
{
    pe_addr_t state=PE_LoadU32(0x8009D2C8u);
    uint32_t selected=PE_LoadU32(payload)&0xFFFFFFu;
    PE_StoreU32(state+0x2Cu,payload);
    uint32_t occupied=audio_music_physical_mask(0x800BA560u,PE_LoadU32(state+0x6Cu));
    audio_or(0x800BCD5Cu,~occupied & ~PE_LoadU32(0x800BCD50u) & 0xFFFFFFu);
    state=PE_LoadU32(0x8009D2C8u);PE_StoreU32(state+0x18u,0);
    if(PE_LoadU32(0x8009D2DCu)&1u) {PE_StoreU32(state+4u,0);audio_or(state+0x1Cu,selected);}
    else {PE_StoreU32(state+0x1Cu,0);audio_or(state+4u,selected);}
    PE_StoreU32(state+8u,PE_LoadU32(payload+4u)&0xFFFFFFu);
    PE_StoreU32(state+12u,PE_LoadU32(payload+8u)&0xFFFFFFu);
    PE_StoreU32(state,(PE_LoadU32(state)&~0x101u)|(PE_LoadU32(0x8009CDE8u)&0x100u));
    pe_addr_t offsets=payload+16u;
    static const uint16_t zeros[]={0xE0,0xDE,0x82,0xE4,0x7A,0xD2,0xD0,0x78,0x82,0x74,0x72,0x84,0xEC,0xCE,0xB4,0xA6,0x94,0xB6,0xA8,0x96,0xBC,0xBA};
    for(unsigned i=0;i<24;i++) {
        pe_addr_t voice=0x800B8AC0u+i*0x11Cu;
        if(selected&(1u<<i)) {
            uint16_t offset=PE_LoadU16(offsets);offsets+=2;
            PE_StoreU32(voice,offsets+offset);
            PE_StoreU32(voice+0xF0u,24);PE_StoreU16(voice+0x56u,4);PE_StoreU16(voice+0x58u,2);
            PE_StoreU16(voice+0x6Cu,0x7F00u);PE_StoreU32(voice+0x44u,0x3FFF0000u);
            PE_StoreU16(voice+0xD8u,0x4000u);
            for(unsigned j=0;j<sizeof(zeros)/sizeof(zeros[0]);j++)PE_StoreU16(voice+zeros[j],0);
            PE_StoreU32(voice+0x34u,0);PE_StoreU32(voice+0x38u,0);
            PE_StoreU16(voice+0x76u,0x8000u);PE_StoreU32(voice+0x14u,payload);
            audio_instrument_init(voice);
        } else {
            PE_StoreU16(voice+0x56u,3);PE_StoreU16(voice+0x58u,1);
            PE_StoreU32(voice,0x8009B8F4u);PE_StoreU16(voice+0x116u,5);audio_or(voice+0xF4u,0x4400u);
        }
    }
    state=PE_LoadU32(0x8009D2C8u);
    PE_StoreU32(state+0x20u,0xFFFF0000u);PE_StoreU32(state+0x28u,1);
    static const uint16_t half_zero[]={0x52,0x58,0x62,0x60,0x5E,0x64,0x56};
    static const uint16_t word_zero[]={0x40,0x44,0x34,0x38,0x3C,0x10,0x30};
    for(unsigned i=0;i<sizeof(half_zero)/sizeof(half_zero[0]);i++)PE_StoreU16(state+half_zero[i],0);
    for(unsigned i=0;i<sizeof(word_zero)/sizeof(word_zero[0]);i++)PE_StoreU32(state+word_zero[i],0);
    PE_StoreU32(state+0x14u,0xFFFFFFu);PE_StoreU32(0x8009D2C4u,0);audio_dirty();
}

/* Complete original 8AC40: restore the cached primary song and relocate it. */
static void audio_music_restore(pe_addr_t payload)
{
    audio_copy(0x800B8968u,PE_LoadU32(0x8009D2C8u),0x68u);
    audio_copy(0x800B6B80u,0x800B8AC0u,0x1AA0u);
    pe_addr_t state=PE_LoadU32(0x8009D2C8u);
    PE_StoreU32(state+0x2Cu,payload);audio_or(state,PE_LoadU32(0x8009CDE8u)&0x100u);
    PE_StoreU32(state+0x10u,PE_LoadU32(state+0x14u));
    uint32_t delta=payload-PE_LoadU32(0x800B8994u);audio_or(0x8009D2C4u,0x90u);
    for(unsigned i=0;i<24;i++) {
        pe_addr_t voice=0x800B8AC0u+i*0x11Cu;
        if(PE_LoadU32(state+4u)&(1u<<i)) {
            static const uint16_t relocated[]={0,0x14,4,8,12,16};
            for(unsigned j=0;j<sizeof(relocated)/sizeof(relocated[0]);j++)PE_StoreU32(voice+relocated[j],PE_LoadU32(voice+relocated[j])+delta);
            PE_StoreU16(voice+0x56u,PE_LoadU16(voice+0x56u)+2u);PE_StoreU16(voice+0x58u,PE_LoadU16(voice+0x58u)+2u);
            audio_or(voice+0xF4u,0x1FF93u);
            if((PE_LoadU32(state)&0x100u) && PE_LoadU16(voice+0x5Au)>=0x20u)PE_StoreU16(voice+0x5Au,PE_LoadU16(voice+0x5Au)+0x30u);
        } else {
            PE_StoreU16(voice+0x58u,2);PE_StoreU16(voice+0x56u,4);PE_StoreU32(voice,0x8009B8F4u);
        }
    }
    state=PE_LoadU32(0x8009D2C8u);
    uint32_t occupied=audio_music_physical_mask(0x800BA560u,PE_LoadU32(state+0x6Cu)&PE_LoadU32(state+0x70u));
    PE_StoreU32(state+0x18u,0);audio_or(0x800BCD5Cu,~occupied & ~PE_LoadU32(0x800BCD50u)&0xFFFFFFu);
    audio_dirty();PE_StoreU16(0x800B89BCu,0);
    if(PE_LoadU32(0x8009D2DCu)&1u) {PE_StoreU32(state+0x1Cu,PE_LoadU32(state+4u));PE_StoreU32(state+4u,0);}
}

static void audio_stop_music(uint32_t id, pe_addr_t voices)
{
    pe_addr_t state = PE_LoadU32(0x8009D2C8u);
    unsigned i;
    if (!id) {
        if (!PE_LoadU32(state + 4u)) return;
    } else if (id != PE_LoadU16(state + 0x54u)) {
        return;
    }
    PE_StoreU32(state + 0x18u, 0xFFFFFFu);
    for (i = 0; i < 24; i++) {
        pe_addr_t voice = voices + i * 0x11Cu;
        PE_StoreU16(voice + 0x56u, 3u);
        PE_StoreU16(voice + 0x58u, 1u);
        PE_StoreU32(voice, 0x8009B8F4u);
        PE_StoreU16(voice + 0x116u, 5u);
        audio_or(voice + 0xF4u, 0x4400u);
    }
}

/* 8AB9C/8ABF0 and the mark loops in 8C814/8C9DC. */
static void audio_mark_voices(uint32_t mask, uint32_t bit,
                              pe_addr_t voices, uint32_t flags)
{
    for (; mask && bit; bit <<= 1, voices += 0x11Cu) {
        if (mask & bit) {
            audio_or(voices + 0xF4u, flags);
            mask ^= bit;
        }
    }
}

static int audio_command(pe_addr_t callback, pe_addr_t entry)
{
    pe_addr_t state = PE_LoadU32(0x8009D2C8u);
    uint32_t mask, i, bit, owner;
    switch (callback) {
    case 0x8008AE94u: case 0x8008B040u: case 0x8008AFB8u: {
        uint32_t id=PE_LoadU32(entry+12u);
        if(callback==0x8008AFB8u) {
            if(PE_LoadU32(state+4u) && !PE_LoadU32(state+0x6Cu)) {
                audio_copy(state,state+0x68u,0x68u);
                audio_copy(0x800B8AC0u,0x800BA560u,0x1AA0u);
            }
            audio_music_new(PE_LoadU32(entry+4u));
            PE_StoreU16(PE_LoadU32(0x8009D2C8u)+0x54u,(uint16_t)id);
        } else {
            uint16_t cached=PE_LoadU16(0x800B89BCu);
            if(cached && cached==id)audio_music_restore(PE_LoadU32(entry+4u));
            else {audio_music_new(PE_LoadU32(entry+4u));PE_StoreU16(PE_LoadU32(0x8009D2C8u)+0x54u,(uint16_t)id);}
            if(callback==0x8008B040u) {uint32_t count=PE_LoadU32(entry+16u);PE_StoreU32(0x8009D22Cu,count?count-1u:0);}
        }
        break;
    }
    case 0x8008CA7Cu: return 1; /* Original two-instruction empty callback. */
    case 0x8008C374u:
        audio_stop_music(0u, 0x800B8AC0u);
        PE_StoreU32(0x8009D2C8u, state + 0x68u);
        audio_stop_music(0u, 0x800BA560u);
        PE_StoreU32(0x8009D2C8u, state);
        break;
    case 0x8008C3E4u: {
        uint32_t id = PE_LoadU32(entry + 4u);
        audio_stop_music(id, 0x800B8AC0u);
        if (id) {
            PE_StoreU32(0x8009D2C8u, state + 0x68u);
            audio_stop_music(id, 0x800BA560u);
            PE_StoreU32(0x8009D2C8u, state);
        }
        break;
    }
    case 0x8008C46Cu:
        for (i = 0, bit = 0x1000u; i < 12; i++, bit <<= 1) {
            pe_addr_t voice = 0x800BC000u + i * 0x11Cu;
            if ((PE_LoadU32(0x800BCD50u) & bit) &&
                !(PE_LoadU32(voice + 0x2Cu) & 0x2000000u)) {
                audio_or(0x800BCD5Cu, bit);
                audio_release(voice, bit);
                PE_StoreU32(voice + 0x38u, 0u);
            }
        }
        audio_or(0x8009D2C4u, 0x10u); audio_dirty(); break;
    case 0x8008C814u:
        mask = PE_LoadU32(state + 0x1Cu);
        if (mask) {
            audio_mark_voices(mask, 1u, 0x800B8AC0u, 0x2B13u);
            PE_StoreU32(state + 0x1Cu, 0u); PE_StoreU32(state + 4u, mask);
            audio_dirty();
        }
        audio_and(0x8009D2DCu, ~1u); break;
    case 0x8008C9DCu:
        mask = PE_LoadU32(0x800BCD60u);
        if (mask) {
            audio_mark_voices(mask, 0x1000u, 0x800BC000u, 0x2B13u);
            PE_StoreU32(0x800BCD60u, 0u); PE_StoreU32(0x800BCD50u, mask);
            audio_dirty();
        }
        audio_and(0x8009D2DCu, ~2u); break;
    case 0x8008C55Cu: case 0x8008C654u: case 0x8008C5D8u:
        PE_StoreU32(0x8009D2C0u, callback == 0x8008C55Cu ? 1u :
                    (callback == 0x8008C654u ? 2u : 4u));
        audio_mark_voices(PE_LoadU32(state + 4u), 1u, 0x800B8AC0u, 3u);
        PE_StoreU32(0x8009D2C8u, state + 0x68u);
        audio_mark_voices(PE_LoadU32(state + 0x6Cu), 1u, 0x800BA560u, 3u);
        PE_StoreU32(0x8009D2C8u, state);
        audio_mark_voices(PE_LoadU32(0x800BCD50u), 0x1000u, 0x800BC000u, 3u);
        break;
    case 0x8008B1FCu:
        owner = PE_LoadU32(entry + 16u);
        if (!owner || owner == PE_LoadU16(state + 0x54u)) {
            PE_StoreU32(state + 0x48u, (PE_LoadU32(entry + 4u) & 127u) << 16);
            PE_StoreU16(state + 0x50u, 0u);
            audio_mark_voices(PE_LoadU32(state + 4u), 1u, 0x800B8AC0u, 3u);
        } else if (owner == PE_LoadU16(state + 0xBCu)) {
            PE_StoreU32(0x8009D2C8u, state + 0x68u);
            PE_StoreU16(state + 0xB8u, 0u);
            PE_StoreU32(state + 0xB0u, (PE_LoadU32(entry + 4u) & 127u) << 16);
            audio_mark_voices(PE_LoadU32(state + 0x6Cu), 1u, 0x800BA560u, 3u);
            PE_StoreU32(0x8009D2C8u, state);
        }
        break;
    case 0x8008B2CCu: {
        uint32_t duration = PE_LoadU32(entry + 4u);
        int32_t target = (int32_t)((PE_LoadU32(entry + 8u) & 127u) << 16);
        owner = PE_LoadU32(entry + 16u);
        if (!duration) duration = 1u;
        if (!owner || owner == PE_LoadU16(state + 0x54u)) {
            PE_StoreU16(state + 0x50u, (uint16_t)duration);
            PE_StoreU32(state + 0x4Cu,
                        (uint32_t)((target - (int32_t)PE_LoadU32(state + 0x48u)) /
                                   (int32_t)duration));
            audio_mark_voices(PE_LoadU32(state + 4u), 1u, 0x800B8AC0u, 3u);
        } else if (owner == PE_LoadU16(state + 0xBCu)) {
            PE_StoreU16(state + 0xB8u, (uint16_t)duration);
            PE_StoreU32(state + 0xB4u,
                        (uint32_t)((target - (int32_t)PE_LoadU32(state + 0xB0u)) /
                                   (int32_t)duration));
            PE_StoreU32(0x8009D2C8u, state + 0x68u);
            audio_mark_voices(PE_LoadU32(state + 0x6Cu), 1u, 0x800BA560u, 3u);
            PE_StoreU32(0x8009D2C8u, state);
        }
        break;
    }
    case 0x8008B410u: {
        /* Retail 8B410..8B580: explicit start/end music-volume ramp.
         * Zero duration becomes one before signed division; the countdown
         * stores only the low halfword. Owner zero selects the first slot. */
        uint32_t duration = PE_LoadU32(entry + 4u);
        int32_t first = (int32_t)((PE_LoadU32(entry + 8u) & 127u) << 16);
        int32_t last = (int32_t)((PE_LoadU32(entry + 12u) & 127u) << 16);
        pe_addr_t selected = state, voices = 0x800B8AC0u;
        owner = PE_LoadU32(entry + 16u);
        if (!duration) duration = 1u;
        if (owner && owner != PE_LoadU16(state + 0x54u)) {
            if (owner != PE_LoadU16(state + 0xBCu)) break;
            selected += 0x68u;
            voices = 0x800BA560u;
        }
        PE_StoreU32(selected + 0x48u, (uint32_t)first);
        PE_StoreU16(selected + 0x50u, (uint16_t)duration);
        PE_StoreU32(selected + 0x4Cu,
                    (uint32_t)((last - first) / (int32_t)duration));
        PE_StoreU32(0x8009D2C8u, selected);
        audio_mark_voices(PE_LoadU32(selected + 4u), 1u, voices, 3u);
        PE_StoreU32(0x8009D2C8u, state);
        break;
    }
    case 0x8008B780u: {
        /* A1 / 8B780..8B900: ramp selected effect voices. Group overlap
         * takes precedence over handle equality. The full duration is
         * tested for zero, then its signed low halfword is the divisor. */
        uint32_t active=PE_LoadU32(0x800BCD50u),group=PE_LoadU32(entry+8u);
        for (unsigned i=0;i<12u;i++) {
            pe_addr_t voice=0x800BC000u+i*0x11Cu;
            if (!(active&(0x1000u<<i))) continue;
            if (group ? !(PE_LoadU32(voice+0x2Cu)&group) :
                        PE_LoadU32(voice+0x28u)!=PE_LoadU32(entry+4u)) continue;
            uint32_t duration=PE_LoadU32(entry+12u);
            if (!duration) duration=1u;
            int32_t delta=(int16_t)(((PE_LoadU32(entry+16u)&127u)<<8)-PE_LoadU16(voice+0xD8u));
            int32_t divisor=(int16_t)duration;
            if (!divisor) {PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;}
            PE_StoreU16(voice+0x74u,(uint16_t)duration);
            PE_StoreU16(voice+0xDAu,(uint16_t)(delta/divisor));
        }
        break;
    }
    case 0x8008B978u: {
        /* A9: fade every active, unprotected effect voice. The original
         * tests the full duration for zero, then divides by its signed
         * low halfword. Preserve the BREAK prefix for a zero divisor. */
        uint32_t active=PE_LoadU32(0x800BCD50u);
        for(unsigned i=0;i<12;i++){
            pe_addr_t voice=0x800BC000u+i*0x11Cu;
            if(!(active&(0x1000u<<i)) || (PE_LoadU32(voice+0x2Cu)&0x2000000u))continue;
            uint32_t duration=PE_LoadU32(entry+4u);
            if(!duration)duration=1;
            uint32_t target=(PE_LoadU32(entry+8u)&127u)<<8;
            int32_t delta=(int16_t)(target-PE_LoadU16(voice+0xD8u));
            int32_t divisor=(int16_t)duration;
            if(!divisor){PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;}
            PE_StoreU16(voice+0x74u,(uint16_t)duration);
            PE_StoreU16(voice+0xDAu,(uint16_t)(delta/divisor));
        }
        break;
    }
    case 0x8008B698u: case 0x8008BA3Cu: {
        /* Original A0/A2: active effect voices, selected by group overlap
         * when group != 0, otherwise by exact playback handle. */
        uint32_t group = PE_LoadU32(entry + 8u);
        uint32_t active = PE_LoadU32(0x800BCD50u);
        uint32_t handle = PE_LoadU32(entry + 4u);
        for (unsigned i = 0; i < 12u; i++) {
            pe_addr_t voice = 0x800BC000u + i * 0x11Cu;
            if (!(active & (0x1000u << i))) continue;
            if (group ? !(PE_LoadU32(voice + 0x2Cu) & group) :
                         PE_LoadU32(voice + 0x28u) != handle) continue;
            if (callback == 0x8008B698u) {
                PE_StoreU16(voice + 0x74u, 0u);
                PE_StoreU16(voice + 0xD8u,
                            (uint16_t)((PE_LoadU32(entry + 12u) & 127u) << 8));
            } else {
                PE_StoreU16(voice + 0x78u, 0u);
                PE_StoreU16(voice + 0x76u, (uint16_t)(PE_LoadU8(entry + 12u) << 8));
            }
            audio_or(voice + 0xF4u, 3u);
        }
        break;
    }
    case 0x8008B1D0u:
        audio_stop(PE_LoadU32(entry + 4u), PE_LoadU32(entry + 8u)); break;
    case 0x8008B168u: {
        pe_addr_t header = PE_LoadU32(entry + 4u);
        uint16_t first = PE_LoadU16(header), second = PE_LoadU16(header + 2u);
        PE_StoreU32(entry + 4u, PE_LoadU32(entry + 20u));
        audio_start(entry, first == 0xFFFFu ? 0u : header + first + 4u,
                    second == 0xFFFFu ? 0u : header + second + 4u);
        break;
    }
    case 0x8008C70Cu:
        PE_StoreU16(state + 0x56u, (uint16_t)PE_LoadU32(entry + 4u)); break;
    default:
        fprintf(stderr, "[STUB:BOOTSTRAP_RET] func_8008CA84_command callback=0x%08X cmd=0x%02X\n",
                (unsigned)callback, (unsigned)PE_LoadU8(entry));
        (void)Bootstrap_ReturnInt4Indirect("func_8008CA84_command", "func_8008CA84",
            0, callback, entry, PE_LoadU8(entry), 0u, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    return 1;
}

void func_8008CA84(void)
{
    pe_addr_t entry = 0x800B8628u;
    while (PE_LoadU32(0x8009D2F4u)) {
        pe_addr_t callback = PE_LoadU32(0x8009C0C0u + PE_LoadU8(entry) * 4u);
        if (!audio_command(callback, entry)) return;
        PE_StoreU32(0x8009D2F4u, PE_LoadU32(0x8009D2F4u) - 1u);
        entry += 0x24u;
    }
}
