/*
 * pe_plat audio output -> host_audio device/WAV sink (step P0 adapter).
 *
 * Kept apart from plat_audio.c because host_audio.c links only into the
 * windowed executable (it needs the writer thread and dlopen).
 */
#include "pe_plat/audio.h"

#include "host_audio.h"

static void plat_host_submit(void *ctx, const int16_t *stereo, int frames)
{
    (void)ctx;
    HostAudio_Submit(stereo, frames);
}

static void plat_host_ff(void *ctx, int on)
{
    (void)ctx;
    HostAudio_SetFastForward(on);
}

static void plat_host_close(void *ctx)
{
    (void)ctx;
    HostAudio_Close();
}

static const PePlatAudioOutput plat_host_output = {
    NULL, plat_host_submit, plat_host_ff, plat_host_close, "host_audio"
};

int pe_plat_audio_open_host_output(int want_device)
{
    if (!HostAudio_Open(want_device)) return 0;
    pe_plat_audio_set_output(&plat_host_output);
    return 1;
}
