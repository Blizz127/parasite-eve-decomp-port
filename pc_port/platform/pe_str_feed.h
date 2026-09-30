#ifndef PE_STR_FEED_H
#define PE_STR_FEED_H

/*
 * HOST_ADAPTED STR stream feeder (fmv lane).
 *
 * Production runs do not enable the modeled CD register device
 * (pe_cdreg.c), so the retail streaming library's IRQ-driven sector
 * assembly (7C13C → 7C564 → DMA3 → 7C214) never fills the ring that
 * StGetNext (func_8007C484) polls.  When CdReadS (func_80081314) opens a
 * stream with the device disabled, this feeder takes over the *producer*
 * half of the libcd stream ring: it reads raw sectors from the active
 * disc at the 2x drive rate (150 sectors/s = 2.5 per VBlank), assembles
 * STR video chunks into the StSetRing records/data exactly where
 * func_8007C484 / func_8007C394 look for them, and hands XA-ADPCM audio
 * sectors to the audio lane's SPU CD input (PE_Spu_CdSector).  Everything downstream (the guest movie
 * players, DecDCTvlc, MDEC, display) is unchanged.
 *
 * Time: the feeder advances emulated VBlanks (HostFB_VSync(0), plus one
 * HostFB_Present refresh when the guest has not presented since the
 * previous VBlank) only while the guest is waiting on the stream, which
 * is when retail's CPU is waiting on the drive.
 */

#include <stdint.h>

#include "pe_guest_ram.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 1 when the sector at the BCD CdlLOC is an STR video or XA audio sector
 * on the active disc (the feeder only takes over real movie streams). */
int  PE_StrFeed_Probe(pe_addr_t bcd_location);
/* Forget all feeder state (host run-control reset). */
void PE_StrFeed_Reset(void);
/* Called by func_80081314 with the device disabled. */
void PE_StrFeed_Open(pe_addr_t bcd_location, uint32_t mode);
/* Called by func_8007A2A4 (stream teardown) and CdlPause/Stop paths. */
void PE_StrFeed_Close(void);
int  PE_StrFeed_Active(void);
/* Called at the top of func_8007C484: make the next frame available in
 * the ring (reading sectors / advancing VBlanks as the drive would). */
void PE_StrFeed_Pump(void);

typedef struct {
    uint32_t start_lba, lba;
    uint32_t sectors, video_sectors, audio_sectors, other_sectors;
    uint32_t frames_published, frames_dropped, vblanks;
    uint32_t last_frame;
} PeStrFeedStats;
void PE_StrFeed_GetStats(PeStrFeedStats *out);

#ifdef __cplusplus
}
#endif

#endif
