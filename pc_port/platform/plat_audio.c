/*
 * pe_plat audio -> in-house pe_spu synthesizer (step P0 adapter).
 *
 * Voice control goes through the same register file the translated driver
 * uses (pe_spu_dma.c), so the synthesizer observes it exactly as it does
 * the game's own writes.  Register offsets are the documented SPU layout
 * (psx-spx): 0x188/0x18A key-on, 0x18C/0x18E key-off.
 */
#include "pe_plat/audio.h"

#include <string.h>

#include "pe_spu.h"
#include "pe_spu_dma.h"
#include "pe_mmio.h"

static const PePlatAudioOutput *plat_audio_out;
static int plat_audio_prev_active = -1;

int pe_plat_audio_init(void)
{
    if (plat_audio_prev_active < 0) plat_audio_prev_active = PE_Spu_Active();
    PE_Spu_SetActive(1);
    return 1;
}

void pe_plat_audio_set_output(const PePlatAudioOutput *out)
{
    plat_audio_out = out;
}

void pe_plat_audio_shutdown(void)
{
    const PePlatAudioOutput *o = plat_audio_out;
    plat_audio_out = NULL;
    if (o && o->close) o->close(o->ctx);
    if (plat_audio_prev_active >= 0) {
        PE_Spu_SetActive(plat_audio_prev_active);
        plat_audio_prev_active = -1;
    }
}

void pe_plat_audio_reset(void) { PE_Spu_Reset(); }
int  pe_plat_audio_sample_rate(void) { return PE_SPU_RATE; }

static void plat_audio_key(uint32_t lo_reg, uint32_t mask)
{
    mask &= 0xFFFFFFu;
    if (mask & 0xFFFFu) PE_SpuRegister_StoreU16(lo_reg, (uint16_t)mask);
    if (mask >> 16)     PE_SpuRegister_StoreU16(lo_reg + 2u, (uint16_t)(mask >> 16));
}

void pe_plat_audio_voice_key_on(uint32_t voice_mask)  { plat_audio_key(0x188u, voice_mask); }
void pe_plat_audio_voice_key_off(uint32_t voice_mask) { plat_audio_key(0x18Cu, voice_mask); }

int pe_plat_audio_transfer_write(uint32_t address, const void *data, size_t len)
{
    uint8_t *ram = PE_SpuRam_Data();
    const uint8_t *src = (const uint8_t *)data;
    size_t i;
    if (!ram || (!data && len)) return 0;
    for (i = 0; i < len; i++)
        ram[(address + i) & (PE_PLAT_AUDIO_SAMPLE_RAM - 1u)] = src[i];
    return 1;
}

int pe_plat_audio_transfer_read(uint32_t address, void *out, size_t len)
{
    const uint8_t *ram = PE_SpuRam_Data();
    uint8_t *dst = (uint8_t *)out;
    size_t i;
    if (!ram || (!out && len)) return 0;
    for (i = 0; i < len; i++)
        dst[i] = ram[(address + i) & (PE_PLAT_AUDIO_SAMPLE_RAM - 1u)];
    return 1;
}

void pe_plat_audio_cd_push(const int16_t *stereo, int frames)
{
    if (stereo && frames > 0) PE_Spu_PushCdAudio(stereo, frames);
}

void pe_plat_audio_cd_stop(void) { PE_Spu_CdInputStop(); }

void pe_plat_audio_mix(int16_t *stereo_out, int frames)
{
    if (stereo_out && frames > 0) PE_Spu_Render(stereo_out, frames);
}

void pe_plat_audio_tick(int frames)
{
    int16_t buf[2 * 512];
    while (frames > 0) {
        int n = frames > 512 ? 512 : frames;
        PE_Spu_Render(buf, n);
        if (plat_audio_out && plat_audio_out->submit)
            plat_audio_out->submit(plat_audio_out->ctx, buf, n);
        frames -= n;
    }
}

void pe_plat_audio_set_fast_forward(int on)
{
    if (plat_audio_out && plat_audio_out->set_fast_forward)
        plat_audio_out->set_fast_forward(plat_audio_out->ctx, on);
}

void pe_plat_audio_stats(PePlatAudioStats *out)
{
    PeSpuStats s;
    if (!out) return;
    PE_Spu_GetStats(&s);
    memset(out, 0, sizeof(*out));
    out->key_on_events = s.kon_events;
    out->key_off_events = s.koff_events;
    out->mixed_frames = s.rendered_frames;
    out->voices_active = s.voices_active;
    out->cd_underruns = s.cd_underruns;
}

void pe_plat_audio_commit_registers(void)
{
    PE_MMIO_Commit();   /* host-pointer register writes (pe_mmio.h) */
}
